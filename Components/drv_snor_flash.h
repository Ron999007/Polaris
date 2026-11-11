/**
 * @file    drv_snor_flash.h
 * @brief   W25Q256 SPI Flash Driver Header
 */

#ifndef DRV_SNOR_FLASH_H
#define DRV_SNOR_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "cmsis_os2.h"

/**
 * @brief SNOR Flash Page, Sector, and Chip Size Definitions for W25Q256
 */
#define W25Q256_PAGE_SIZE       256
#define W25Q256_SECTOR_SIZE     4096      /* 4KB */
#define W25Q256_BLOCK32_SIZE    32768     /* 32KB */
#define W25Q256_BLOCK64_SIZE    65536     /* 64KB */
#define W25Q256_FLASH_SIZE      0x2000000 /* 32 MB */

/**
 * @brief W25Q256 Instruction Set (Including 4-Byte Address commands)
 */
#define W25Q_CMD_WRITE_ENABLE           0x06
#define W25Q_CMD_READ_STAT_REG1         0x05
#define W25Q_CMD_READ_JEDEC_ID          0x9F

/* 4-Byte Address Instructions for capacities > 16MB */
#define W25Q_CMD_READ_DATA_4B           0x13  /* Standard Read with 4-Byte Address */
#define W25Q_CMD_PAGE_PROG_4B           0x12  /* Page Program with 4-Byte Address */
#define W25Q_CMD_SECTOR_ERASE_4K_4B     0x21  /* Sector Erase (4KB) with 4-Byte Address */
#define W25Q_CMD_CHIP_ERASE             0xC7

/**
 * @brief Status codes for SNOR Flash operations.
 */
typedef enum {
    SNOR_OK = 0,
    SNOR_ERR,
    SNOR_TIMEOUT,
    SNOR_PARAM_ERR
} snor_status_t;

/**
 * @brief Initializes the SNOR Flash and sets up the FreeRTOS mutex.
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_init(void);

/**
 * @brief Reads the JEDEC ID (Manufacturer ID + Memory Type + Capacity).
 * @param pID Pointer to store the 24-bit ID (e.g., 0xEF4019 for W25Q256).
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_read_id(uint32_t *pID);

/**
 * @brief Reads data from the Flash memory using standard SPI and 4-Byte addressing.
 * @param address The 32-bit start address to read from.
 * @param pData Pointer to the buffer that will receive the data.
 * @param size Number of bytes to read.
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_read(uint32_t address, uint8_t *pData, uint32_t size);

/**
 * @brief Writes a page (up to 256 bytes) to the Flash memory using 4-Byte addressing.
 * @param address The 32-bit start address for writing.
 * @param pData Pointer to the data to be written.
 * @param size Number of bytes (max 256).
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_write_page(uint32_t address, uint8_t *pData, uint32_t size);

/**
 * @brief Erases a 4KB sector using 4-Byte addressing.
 * @param sector_address The 32-bit start address of the sector to be erased.
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_erase_sector(uint32_t sector_address);

/**
 * @brief Erases the entire Flash chip.
 * @return snor_status_t SNOR_OK if successful.
 */
snor_status_t drv_snor_erase_chip(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_SNOR_FLASH_H */
