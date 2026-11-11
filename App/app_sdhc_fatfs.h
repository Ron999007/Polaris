/**
 * @file app_sdhc_fatfs.h
 * @brief Application layer for File System (FatFS) operations header.
 */

#ifndef APP_SDHC_FATFS_H
#define APP_SDHC_FATFS_H

/**
 * @brief FreeRTOS task for FatFS management.
 */
void vTask_FatFS(void *pvParameters);

#endif /* APP_SDHC_FATFS_H */
