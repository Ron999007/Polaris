#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "FreeRTOS.h"
#include "task.h"
#include "port_led.h" /* For system_led_id_e */

/* Define explicit commands for the LED Task */
typedef enum {
    LED_CMD_TURN_OFF = 0,
    LED_CMD_TURN_ON,
    LED_CMD_START_BLINK,
    LED_CMD_START_BREATHE,
	LED_CMD_CYCLE_MODE
} led_cmd_e;

/* Entry point for the LED task */
void vTask_LED(void *pvParameters);

/**
 * @brief  Thread-safe API for other tasks to control LEDs.
 * @param  id   The target LED identifier.
 * @param  cmd  The desired command/state.
 * @retval pdTRUE if the command was successfully queued, pdFALSE if queue is full.
 */
BaseType_t App_LED_Set_State(system_led_id_e id, led_cmd_e cmd);

#endif /* __APP_LED_H__ */
