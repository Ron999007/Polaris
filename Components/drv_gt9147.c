/**
 * @file drv_gt9147.c
 * @brief Implementation of GT9147 Driver with Hardware Reset Sequence
 * @project Polaris
 */

#include "drv_gt9147.h"
#include "port_soft_i2c.h"  /* Software I2C Port Layer */
#include "stm32h7xx_hal.h"  /* Adjust based on your specific STM32 family */
#include "FreeRTOS.h"
#include "task.h"

/* --- Hardware Bindings for Reset and Interrupt Pins --- */
/* Based on user specification: T_CS(RST) = PI8, T_PEN(INT) = PH7 */
#define GT_RST_PORT     GPIOI
#define GT_RST_PIN      GPIO_PIN_8

#define GT_INT_PORT     GPIOH
#define GT_INT_PIN      GPIO_PIN_7

/**
 * @brief Internal wrapper for I2C Memory Read via Port Layer
 * @param reg 16-bit register address
 * @param buf Pointer to data buffer
 * @param len Number of bytes to read
 * @return 0 on success, 1 on error
 */
static uint8_t drv_gt9147_i2c_read(uint16_t reg, uint8_t *buf, uint16_t len) {
    return Port_SoftI2C_Mem_Read(GT9147_I2C_ADDR, reg, buf, len);
}

/**
 * @brief Internal wrapper for I2C Memory Write via Port Layer
 * @param reg 16-bit register address
 * @param buf Pointer to data buffer
 * @param len Number of bytes to write
 * @return 0 on success, 1 on error
 */
static uint8_t drv_gt9147_i2c_write(uint16_t reg, uint8_t *buf, uint16_t len) {
    return Port_SoftI2C_Mem_Write(GT9147_I2C_ADDR, reg, buf, len);
}

/**
 * @brief Initializes GT9147 hardware (Reset, I2C Address Config, Soft Reset)
 */
drv_gt9147_status_t drv_gt9147_init(void) {
    uint8_t temp[4];
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 0. Initialize Software I2C GPIOs (SCL, SDA) via Port Layer */
    Port_SoftI2C_Init();

    /* 1. Enable Clocks for RST (GPIOI) and INT (GPIOH) */
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* 2. Configure RST (PI8) as Output Push-Pull */
    GPIO_InitStruct.Pin = GT_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GT_RST_PORT, &GPIO_InitStruct);

    /* 3. Configure INT (PH7) as Output Push-Pull (Temporarily for address config) */
    GPIO_InitStruct.Pin = GT_INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GT_INT_PORT, &GPIO_InitStruct);

    /* --- STEP 1: Hardware Reset & Address Selection (0x14) --- */
    /* Pull RST and INT LOW simultaneously to initiate reset */
    HAL_GPIO_WritePin(GT_RST_PORT, GT_RST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GT_INT_PORT, GT_INT_PIN, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(20)); /* Delay > 10ms */

    /* Release RST while INT is LOW to select I2C address 0x14 (0x28 for HAL) */
    HAL_GPIO_WritePin(GT_RST_PORT, GT_RST_PIN, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(10)); /* Delay > 5ms */

    /* Reconfigure INT pin to Floating Input to allow GT9147 to drive it */
    GPIO_InitStruct.Pin = GT_INT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GT_INT_PORT, &GPIO_InitStruct);
    vTaskDelay(pdMS_TO_TICKS(100)); /* Allow IC to complete boot process */

    /* --- STEP 2: Software Reset via Command Register --- */
    temp[0] = 0x02; /* Issue Soft Reset command */
    drv_gt9147_i2c_write(GT9147_REG_COMMAND, temp, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    temp[0] = 0x00; /* Resume Normal Operation mode */
    drv_gt9147_i2c_write(GT9147_REG_COMMAND, temp, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* --- STEP 3: Verification --- */
    /* Read Product ID (e.g., "9147" in ASCII) to ensure I2C communication is working */
    if (drv_gt9147_i2c_read(GT9147_REG_PRODUCT_ID, temp, 4) != 0) {
        return DRV_GT9147_ERROR;
    }

    return DRV_GT9147_OK;
}

/**
 * @brief Reads touch coordinates and status from the controller
 */
drv_gt9147_status_t drv_gt9147_read_touch(drv_gt9147_point_t *point) {
    uint8_t status_reg = 0;
    uint8_t clear_reg = 0;
    uint8_t raw_data[4];

    /* Read status register to check if new touch data is ready */
    if (drv_gt9147_i2c_read(GT9147_REG_STATUS, &status_reg, 1) != 0) {
        return DRV_GT9147_ERROR;
    }

    /* Check Bit 7: Buffer Status. If 0, no new touch event has occurred */
    if ((status_reg & 0x80) == 0) {
        return DRV_GT9147_NO_TOUCH;
    }

    /* Check Bit 0-3: Number of touch points currently detected */
    if ((status_reg & 0x0F) > 0) {
        /* Read X/Y coordinates of the first touch point (4 bytes) */
        if (drv_gt9147_i2c_read(GT9147_REG_POINT1_X_L, raw_data, 4) == 0) {
            point->x = ((uint16_t)raw_data[1] << 8) | raw_data[0];
            point->y = ((uint16_t)raw_data[3] << 8) | raw_data[2];
            point->is_pressed = 1;
        }
    } else {
        point->is_pressed = 0; /* Touch released */
    }

    /* CRITICAL: Must write 0 to status register to clear the ready flag.
     * If not cleared, GT9147 will not output subsequent touch events.
     */
    drv_gt9147_i2c_write(GT9147_REG_STATUS, &clear_reg, 1);

    return DRV_GT9147_OK;
}
