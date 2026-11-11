#ifndef APP_SDRAM_H
#define APP_SDRAM_H

#include "FreeRTOS.h"
#include "task.h"

/* Exported functions ------------------------------------------------------- */

/**
 * @brief  FreeRTOS task for SDRAM initialization and memory integrity testing.
 * @param  pvParameters: Task parameters (not used)
 */
void vTask_SDRAM(void *pvParameters);

#endif /* APP_SDRAM_H */
