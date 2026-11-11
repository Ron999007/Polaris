#include "app_led.h"
#include "queue.h"

/* Internal message structure for the Queue */
typedef struct {
    system_led_id_e id;
    led_cmd_e       cmd;
} led_msg_t;

/* Private global variables (Encapsulated) */
static drv_led_t     s_system_leds[LED_ID_MAX];
static QueueHandle_t s_led_queue = NULL;

/* ====================================================================
 * Public Thread-Safe API
 * ==================================================================== */
BaseType_t App_LED_Set_State(system_led_id_e id, led_cmd_e cmd)
{
    /* Protect against calling before queue is created */
    if (s_led_queue == NULL) return pdFALSE;

    led_msg_t msg;
    msg.id = id;
    msg.cmd = cmd;

    /* Send message to the back of the queue with 0 timeout (don't block the caller) */
    return xQueueSend(s_led_queue, &msg, 0);
}

/* ====================================================================
 * Main LED Task
 * ==================================================================== */
void vTask_LED(void *pvParameters)
{
    led_msg_t received_msg;

    /* 1. Initialize hardware bindings */
    Port_LED_Hardware_Init(s_system_leds);

    /* 2. Create the OS Message Queue (holds up to 10 commands) */
    s_led_queue = xQueueCreate(10, sizeof(led_msg_t));
    if (s_led_queue == NULL) {
        /* Failed to create queue, halt task */
        vTaskDelete(NULL);
    }

    /* 3. Initial states */
    drv_led_set_off(&s_system_leds[LED_ID_0]);
    drv_led_set_off(&s_system_leds[LED_ID_1]);

    /* 4. Infinite OS Task Loop */
    for(;;)
    {
        /* Wait for commands OR timeout every 10ms (Replaces vTaskDelay) */
        if (xQueueReceive(s_led_queue, &received_msg, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            /* A command was received, process it safely inside this task */
            if (received_msg.id < LED_ID_MAX)
            {
                drv_led_t *target_led = &s_system_leds[received_msg.id];

                switch (received_msg.cmd) {
                    case LED_CMD_TURN_OFF:
                        drv_led_set_off(target_led);
                        break;
                    case LED_CMD_TURN_ON:
                        drv_led_set_on(target_led, 100);
                        break;
                    case LED_CMD_START_BLINK:
                        drv_led_set_blink(target_led, 200, 100);
                        break;
                    case LED_CMD_START_BREATHE:
                        drv_led_set_breathe(target_led, 50, 100);
                        break;

                    /* Cycle logic: OFF -> ON -> BLINK -> BREATHE -> OFF */
                    case LED_CMD_CYCLE_MODE:
                    {
                        led_mode_e current_mode = drv_led_get_current_mode(target_led);
                        switch (current_mode) {
                            case LED_MODE_OFF:
                                drv_led_set_on(target_led, 100);
                                break;
                            case LED_MODE_ON:
                                drv_led_set_blink(target_led, 200, 100);
                                break;
                            case LED_MODE_BLINK:
                                drv_led_set_breathe(target_led, 50, 100);
                                break;
                            case LED_MODE_BREATHE:
                            default:
                                drv_led_set_off(target_led);
                                break;
                        }
                        break;
                    }
                }
            }
        }

        /* Update animation engine for ALL managed LEDs every tick */
        for (int i = 0; i < LED_ID_MAX; i++) {
            drv_led_tick_handler(&s_system_leds[i], 10);
        }
    }
}
