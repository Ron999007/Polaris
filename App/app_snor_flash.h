/**
 * @file app_snor_flash.h
 * @brief FreeRTOS Task for W25Q64JV Full-Chip Test
 * @project Polaris
 *
 *  Created on: 2026年4月24日
 *      Author: ronchen
 */

#ifndef APP_SNOR_FLASH_H_
#define APP_SNOR_FLASH_H_

/**
 * @brief  FreeRTOS task entry point for the SNOR flash full-chip test.
 *         - Initialises the flash driver.
 *         - Waits for SnorTestHandle semaphore (triggered by Touch UI).
 *         - Sequentially erases, writes, and reads-back every sector.
 *         - Reports pass/fail via UART printf.
 *         - Self-deletes on completion.
 *
 * @param  pvParameters  Unused (FreeRTOS task argument).
 */
void vTask_SNOR(void *pvParameters);

#endif /* APP_SNOR_FLASH_H_ */
