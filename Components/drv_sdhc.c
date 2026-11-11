/**
 * @file drv_sdhc.c
 * @brief SDHC (SDMMC1) hardware driver layer implementation.
 */

#include "drv_sdhc.h"
#include <stdio.h>

/* Global handle for SDMMC1 */
extern SD_HandleTypeDef hsd1;

/**
 * @brief Initializes the SDMMC1 peripheral, GPIOs, and the SD card.
 * Replaces the default MX_SDMMC1_SD_Init to avoid Error_Handler hangs.
 * @return 0 on success, -1 on failure.
 */
int32_t drv_sdhc_init(void) {

    /* SDMMC1 Peripheral Configuration (Formerly MX_SDMMC1_SD_Init logic) */
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 2; /* Adjust based on your 48MHz SDMMC kernel clock */

    /* Call HAL_SD_Init.
     * We return a status instead of calling Error_Handler() to prevent system hang.
     */
    if (HAL_SD_Init(&hsd1) != HAL_OK) {
    	printf("[drv_sdhc] HAL_SD_Init Failed! ErrorCode: 0x%08lX\r\n", hsd1.ErrorCode);
        return -1;
    }

    return 0;
}

/**
 * @brief Retrieves the pointer to the SD handle.
 * @return Pointer to SD_HandleTypeDef.
 */
SD_HandleTypeDef* drv_sdhc_get_handle(void) {
    return &hsd1;
}
