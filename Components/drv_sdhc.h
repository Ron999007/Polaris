/**
 * @file drv_sdhc.h
 * @brief SDHC (SDMMC1) hardware driver layer header.
 * Note: All functions are in lowercase as requested.
 */

#ifndef DRV_SDHC_H
#define DRV_SDHC_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

/**
 * @brief Initializes the SDMMC1 peripheral, GPIOs, and the SD card itself.
 * This includes the logic originally in MX_SDMMC1_SD_Init.
 * @return 0 on success, -1 on failure.
 */
int32_t drv_sdhc_init(void);

/**
 * @brief Retrieves the pointer to the SD handle.
 * @return Pointer to SD_HandleTypeDef.
 */
SD_HandleTypeDef* drv_sdhc_get_handle(void);

#endif /* DRV_SDHC_H */
