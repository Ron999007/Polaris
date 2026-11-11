#ifndef __PORT_LED_H__
#define __PORT_LED_H__

#include "drv_led.h"

/* Definition of LED identifiers for this board */
typedef enum {
    LED_ID_0 = 0, /* Connected to PB1 */
    LED_ID_1,     /* Connected to PB0 */
    LED_ID_MAX
} system_led_id_e;

/**
 * @brief Initialize hardware binding for LED components
 * @param led_array Pointer to the LED object array
 */
void Port_LED_Hardware_Init(drv_led_t *led_array);

void Port_LED_Timer_Handler(void);

#endif /* __PORT_LED_H__ */
