/**
 * @file    drv_snor_flash.c
 * @brief   W25Q256 SPI Flash Driver (OVR Fix & Bulletproof Write)
 */

#include <stdio.h>
#include "drv_snor_flash.h"
#include "spi.h"

/* ==================================================================== */
/* Debug Configuration                                                  */
/* ==================================================================== */
#define ENABLE_FLASH_DEBUG 0  /* Set to 0 to disable all FLASH_PRINTF messages */

#if ENABLE_FLASH_DEBUG
    #define FLASH_PRINTF(...) printf(__VA_ARGS__)
#else
    #define FLASH_PRINTF(...)
#endif

/* Flash Chip Select Pin Definitions */
#define F_CS_GPIO_Port  GPIOF
#define F_CS_Pin        GPIO_PIN_10

#define SNOR_CS_LOW()   HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_RESET)
#define SNOR_CS_HIGH()  HAL_GPIO_WritePin(F_CS_GPIO_Port, F_CS_Pin, GPIO_PIN_SET)

static osMutexId_t snor_mutex = NULL;

static void drv_snor_lock(void) {
    if (snor_mutex) osMutexAcquire(snor_mutex, osWaitForever);
}

static void drv_snor_unlock(void) {
    if (snor_mutex) osMutexRelease(snor_mutex);
}

static void drv_snor_spi_recover(void) {
    if (hspi2.State != HAL_SPI_STATE_READY) {
        __HAL_SPI_CLEAR_OVRFLAG(&hspi2);
        hspi2.State = HAL_SPI_STATE_READY;
    }
}

static HAL_StatusTypeDef drv_snor_spi_transmit_safe(uint8_t *pData, uint16_t size) {
    uint8_t dummy_rx;
    HAL_StatusTypeDef status = HAL_OK;

    drv_snor_spi_recover();

    for (uint16_t i = 0; i < size; i++) {
        status = HAL_SPI_TransmitReceive(&hspi2, &pData[i], &dummy_rx, 1, 10);
        if (status != HAL_OK) break;
    }
    return status;
}

static uint8_t drv_snor_read_sr1(void) {
    uint8_t tx_buf[2] = {W25Q_CMD_READ_STAT_REG1, 0xFF};
    uint8_t rx_buf[2] = {0xFF, 0xFF};

    drv_snor_spi_recover();
    SNOR_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, 2, 100);
    SNOR_CS_HIGH();

    return rx_buf[1];
}

static snor_status_t drv_snor_wait_busy(uint32_t timeout) {
    uint32_t tickstart = osKernelGetTickCount();
    uint8_t status;
    do {
        status = drv_snor_read_sr1();
        if ((status & 0x01) == 0) return SNOR_OK;
        osDelay(1);
    } while ((osKernelGetTickCount() - tickstart) < timeout);

    FLASH_PRINTF("[Flash] [ERROR] Wait BUSY Timeout! (SR1: 0x%02X)\r\n", status);
    return SNOR_TIMEOUT;
}

static snor_status_t drv_snor_write_enable(void) {
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    uint8_t dummy_rx;

    drv_snor_spi_recover();
    SNOR_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi2, &cmd, &dummy_rx, 1, 100);
    SNOR_CS_HIGH();

    uint32_t tickstart = osKernelGetTickCount();
    while ((drv_snor_read_sr1() & 0x02) == 0) {
        if ((osKernelGetTickCount() - tickstart) > 50) {
            FLASH_PRINTF("[Flash] [ERROR] Write Enable Latch (WEL) Failed!\r\n");
            return SNOR_TIMEOUT;
        }
        osDelay(1);
    }
    return SNOR_OK;
}

snor_status_t drv_snor_init(void) {
    if (snor_mutex == NULL) snor_mutex = osMutexNew(NULL);
    SNOR_CS_HIGH();
    return SNOR_OK;
}

snor_status_t drv_snor_read_id(uint32_t *pID) {
    if (pID == NULL) return SNOR_PARAM_ERR;

    uint8_t tx_buf[4] = {W25Q_CMD_READ_JEDEC_ID, 0xFF, 0xFF, 0xFF};
    uint8_t rx_buf[4] = {0};

    drv_snor_lock();
    drv_snor_spi_recover();
    SNOR_CS_LOW();

    if (HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, 4, 100) == HAL_OK) {
        *pID = (uint32_t)((rx_buf[1] << 16) | (rx_buf[2] << 8) | rx_buf[3]);
    } else {
        *pID = 0;
    }

    SNOR_CS_HIGH();
    drv_snor_unlock();
    return SNOR_OK;
}

snor_status_t drv_snor_read(uint32_t address, uint8_t *pData, uint32_t size) {
    if (pData == NULL || size == 0) return SNOR_PARAM_ERR;

    uint8_t cmd_addr[5];
    cmd_addr[0] = W25Q_CMD_READ_DATA_4B;
    cmd_addr[1] = (uint8_t)((address >> 24) & 0xFF);
    cmd_addr[2] = (uint8_t)((address >> 16) & 0xFF);
    cmd_addr[3] = (uint8_t)((address >> 8) & 0xFF);
    cmd_addr[4] = (uint8_t)(address & 0xFF);

    drv_snor_lock();
    drv_snor_spi_recover();
    SNOR_CS_LOW();

    drv_snor_spi_transmit_safe(cmd_addr, 5);

    for (uint32_t i = 0; i < size; i++) {
        uint8_t dummy = 0xFF;
        HAL_SPI_TransmitReceive(&hspi2, &dummy, &pData[i], 1, 10);
    }

    SNOR_CS_HIGH();
    drv_snor_unlock();
    return SNOR_OK;
}

snor_status_t drv_snor_write_page(uint32_t address, uint8_t *pData, uint32_t size) {
    if ((pData == NULL) || (size == 0u) || (size > W25Q256_PAGE_SIZE)) return SNOR_PARAM_ERR;

    uint8_t cmd_addr[5];
    cmd_addr[0] = W25Q_CMD_PAGE_PROG_4B;
    cmd_addr[1] = (uint8_t)((address >> 24) & 0xFF);
    cmd_addr[2] = (uint8_t)((address >> 16) & 0xFF);
    cmd_addr[3] = (uint8_t)((address >> 8) & 0xFF);
    cmd_addr[4] = (uint8_t)(address & 0xFF);

    drv_snor_lock();

    if (drv_snor_write_enable() != SNOR_OK) {
        drv_snor_unlock();
        return SNOR_ERR;
    }

    drv_snor_spi_recover();
    SNOR_CS_LOW();

    drv_snor_spi_transmit_safe(cmd_addr, 5);
    drv_snor_spi_transmit_safe(pData, size);

    SNOR_CS_HIGH();

    snor_status_t status = drv_snor_wait_busy(1000);
    drv_snor_unlock();

    return status;
}

snor_status_t drv_snor_erase_sector(uint32_t sector_address) {
    uint8_t cmd_addr[5];
    cmd_addr[0] = W25Q_CMD_SECTOR_ERASE_4K_4B;
    cmd_addr[1] = (uint8_t)((sector_address >> 24) & 0xFF);
    cmd_addr[2] = (uint8_t)((sector_address >> 16) & 0xFF);
    cmd_addr[3] = (uint8_t)((sector_address >> 8) & 0xFF);
    cmd_addr[4] = (uint8_t)(sector_address & 0xFF);

    drv_snor_lock();

    if (drv_snor_write_enable() != SNOR_OK) {
        drv_snor_unlock();
        return SNOR_ERR;
    }

    drv_snor_spi_recover();
    SNOR_CS_LOW();
    drv_snor_spi_transmit_safe(cmd_addr, 5);
    SNOR_CS_HIGH();

    snor_status_t status = drv_snor_wait_busy(2000);
    drv_snor_unlock();

    return status;
}

snor_status_t drv_snor_erase_chip(void) {
    drv_snor_lock();
    if (drv_snor_write_enable() != SNOR_OK) {
        drv_snor_unlock();
        return SNOR_ERR;
    }

    drv_snor_spi_recover();
    SNOR_CS_LOW();
    uint8_t cmd = W25Q_CMD_CHIP_ERASE;
    drv_snor_spi_transmit_safe(&cmd, 1);
    SNOR_CS_HIGH();

    snor_status_t status = drv_snor_wait_busy(100000);
    drv_snor_unlock();

    return status;
}
