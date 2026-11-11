#include "app_button.h"
#include "port_button.h"
#include "app_led.h" /* Include API to control LEDs safely */
#include "app_can.h"
#include <stdio.h>

/**
 * @brief FreeRTOS Task for handling Button debouncing and event mapping
 */
void vTask_Button(void *pvParameters)
{
    btn_msg_t msg;

    /* Initialize hardware bindings */
    Port_Button_Hardware_Init();

    for(;;)
    {
        /* Process the state machine every 10ms */
        Button_Ticks();

        /* Read events from the Ring Buffer */
        while (Button_Read_Event(&msg))
        {
            if (msg.event == BTN_EVENT_SHORT_CLICK)
            {
                if (msg.id == BTN_ID_LEFT)
                {
                    printf("LEFT Button: Triggering CYCLE mode for LED0\r\n");
                    /* Send command safely to LED Task */
                    App_LED_Set_State(LED_ID_0, LED_CMD_CYCLE_MODE);
                }
                else if (msg.id == BTN_ID_RIGHT)
                {
                    printf("RIGHT Button: Triggering CYCLE mode for LED1\r\n");
                    /* Send command safely to LED Task */
                    App_LED_Set_State(LED_ID_1, LED_CMD_CYCLE_MODE);
                }
                else if (msg.id == BTN_ID_UP)
                {
                    printf("UP Button Short Clicked!\r\n");
                    App_CAN_Send_Test_Msg();
                }
                else if (msg.id == BTN_ID_DOWN)
                {
                    printf("DOWN Button Short Clicked!\r\n");
                }
            }
            else if (msg.event == BTN_EVENT_LONG_PRESS)
            {
                printf("Button %d Long Pressed!\r\n", msg.id);
            }
        }

        /* Block the task for 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
