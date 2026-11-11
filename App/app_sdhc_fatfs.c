/**
 * @file app_sdhc_fatfs.c
 * @brief Application layer for File System (FatFS) with IDMA integration.
 */
#include "app_sdhc_fatfs.h"
#include "drv_sdhc.h"
#include "ff.h"
#include "diskio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

FATFS SDFatFs;

/* ==================================================================== */
/* 🛡️ BSP Interceptors (FatFS Pacifier - Ultimate Hardware Bridge)      */
/* ==================================================================== */

/**
 * @brief Initializes the SD Card hardware (Bypassed, already handled).
 * @retval 0 indicating success.
 */
uint8_t BSP_SD_Init(void) {
    return 0; /* MSD_OK */
}

/**
 * @brief Checks if the SD Card is detected.
 * @retval 1 indicating SD is present.
 */
uint8_t BSP_SD_IsDetected(void) {
    return 1; /* SD_PRESENT */
}

/**
 * @brief Retrieves the SD Card information.
 * @param CardInfo Pointer to the HAL_SD_CardInfoTypeDef structure.
 */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo) {
    HAL_SD_GetCardInfo(drv_sdhc_get_handle(), CardInfo);
}

/**
 * @brief Retrieves the current state of the SD Card. Ensures DMA is finished.
 * @retval 0 if ready (SD_TRANSFER_OK), 1 if busy (SD_TRANSFER_BUSY).
 */
uint8_t BSP_SD_GetCardState(void) {
	if (HAL_SD_GetState(drv_sdhc_get_handle()) == HAL_SD_STATE_READY &&
		HAL_SD_GetCardState(drv_sdhc_get_handle()) == HAL_SD_CARD_TRANSFER) {
		return 0; /* SD_TRANSFER_OK */
	}
	return 1; /* SD_TRANSFER_BUSY */
}

/**
 * @brief Reads block(s) from the SD Card using IDMA.
 * @param pData Pointer to the buffer where data will be stored.
 * @param ReadAddr Sector address to read from.
 * @param NumOfBlocks Number of blocks to read.
 * @retval 0 on success, 1 on failure.
 */
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    if (HAL_SD_ReadBlocks_DMA(drv_sdhc_get_handle(), (uint8_t *)pData, ReadAddr, NumOfBlocks) == HAL_OK) return 0;
    return 1;
}

/**
 * @brief Writes block(s) to the SD Card using IDMA.
 * @param pData Pointer to the data buffer to write.
 * @param WriteAddr Sector address to write to.
 * @param NumOfBlocks Number of blocks to write.
 * @retval 0 on success, 1 on failure.
 */
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    if (HAL_SD_WriteBlocks_DMA(drv_sdhc_get_handle(), (uint8_t *)pData, WriteAddr, NumOfBlocks) == HAL_OK) return 0;
    return 1;
}

/* ==================================================================== */
/* Main FatFS Task                                                      */
/* ==================================================================== */

void vTask_FatFS(void *pvParameters) {
    FRESULT res;
    DWORD fre_clust;
    FATFS *pfs;

    vTaskDelay(pdMS_TO_TICKS(500));

    for(;;) {
        printf("[FatFS Task] Attempting SDHC hardware init...\r\n");
        if (drv_sdhc_init() == 0) {
            HAL_SD_ConfigWideBusOperation(drv_sdhc_get_handle(), SDMMC_BUS_WIDE_4B);
            printf("[FatFS Task] SDHC Hardware Ready!\r\n");
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    /* Target Drive 0:/ for SDHC */
    printf("[FatFS Task] Mounting logical drive 0:/ (SD Card)...\r\n");
    res = f_mount(&SDFatFs, (TCHAR const*)"0:/", 1);

    if (res == FR_OK) {
        printf("[FatFS Task] Mount Success!\r\n");

        res = f_getfree("0:/", &fre_clust, &pfs);
        if (res == FR_OK) {
            DWORD tot_clust = pfs->n_fatent - 2;
            DWORD sector_size = _MIN_SS;
#if _MAX_SS != _MIN_SS
            sector_size = pfs->ssize;
#endif
            DWORD clust_size_kb = (pfs->csize * sector_size) / 1024;
            DWORD tot_mb = (tot_clust * clust_size_kb) / 1024;
            DWORD fre_mb = (fre_clust * clust_size_kb) / 1024;

            DWORD tot_gb_int = tot_mb / 1024;
            DWORD tot_gb_frac = ((tot_mb % 1024) * 100) / 1024;
            DWORD fre_gb_int = fre_mb / 1024;
            DWORD fre_gb_frac = ((fre_mb % 1024) * 100) / 1024;

            printf("\r\n--- SD Card (0:/) Information ---\r\n");
            printf("Total Clusters: %lu\r\n", tot_clust);
            printf("Cluster Size:   %lu KB\r\n", clust_size_kb);
            printf("Total Space:    %lu MB (%lu.%02lu GB)\r\n", tot_mb, tot_gb_int, tot_gb_frac);
            printf("Free Space:     %lu MB (%lu.%02lu GB)\r\n", fre_mb, fre_gb_int, fre_gb_frac);
            printf("---------------------------\r\n\r\n");
        }
    } else {
        printf("[FatFS Task] Mount Failed (Error: %d).\r\n", res);
    }

    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
