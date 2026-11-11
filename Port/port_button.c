#include "port_button.h"
#include "main.h" /* STM32 HAL Library */

/* ====================================================================
 * Hardware Read Wrapper for STM32 HAL
 * ==================================================================== */
static uint8_t stm32_hal_button_read(void* port, uint32_t pin)
{
    /* Cast the generic port and pin to STM32 HAL types */
    GPIO_PinState state = HAL_GPIO_ReadPin((GPIO_TypeDef*)port, (uint16_t)pin);
    return (state == GPIO_PIN_SET) ? 1 : 0;
}

/* ====================================================================
 * Button Hardware Configuration Table
 * ==================================================================== */
static button_t g_board_buttons[] = {
    /* ID           PORT    PIN          ACTIVE_LEVEL */
    { .id = BTN_ID_UP,    .port = GPIOA, .pin = GPIO_PIN_0,  .active_level = 1 }, /* High Active */
    { .id = BTN_ID_DOWN,  .port = GPIOH, .pin = GPIO_PIN_2,  .active_level = 0 }, /* Low Active */
    { .id = BTN_ID_LEFT,  .port = GPIOC, .pin = GPIO_PIN_13, .active_level = 0 }, /* Low Active */
    { .id = BTN_ID_RIGHT, .port = GPIOH, .pin = GPIO_PIN_3,  .active_level = 0 }  /* Low Active */
};

#define BTN_COUNT (sizeof(g_board_buttons) / sizeof(g_board_buttons[0]))

/* ====================================================================
 * Public Porting API
 * ==================================================================== */
void Port_Button_Hardware_Init(void)
{
    /* Pass the configuration array and the STM32 read wrapper to the core driver */
    Button_Init(g_board_buttons, BTN_COUNT, stm32_hal_button_read);
}
