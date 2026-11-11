#include "port_led.h"
#include "main.h" /* STM32 HAL Library */

/* Hardware context for software PWM implementation */
typedef struct {
    void (*set_pin_state)(uint8_t state);
    uint8_t target_brightness;
    uint8_t soft_pwm_counter;
} hw_soft_pwm_ctx_t;

/* GPIO control wrappers for PB1 and PB0 */
/* Assuming Active-Low: 0 turns ON, 1 turns OFF */
static void board_set_led0_pin(uint8_t state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (state > 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
static void board_set_led1_pin(uint8_t state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (state > 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* Internal contexts for 2 LEDs */
static hw_soft_pwm_ctx_t ctx_led0 = { .set_pin_state = board_set_led0_pin };
static hw_soft_pwm_ctx_t ctx_led1 = { .set_pin_state = board_set_led1_pin };

/* HAL interface adaptation */
static void stm32_soft_pwm_init(void *hw_context) {
    hw_soft_pwm_ctx_t *ctx = (hw_soft_pwm_ctx_t *)hw_context;
    ctx->target_brightness = 0;
    ctx->soft_pwm_counter = 0;
    if (ctx->set_pin_state) ctx->set_pin_state(0);
}

static void stm32_soft_pwm_set_brightness(void *hw_context, uint8_t brightness) {
    hw_soft_pwm_ctx_t *ctx = (hw_soft_pwm_ctx_t *)hw_context;
    ctx->target_brightness = brightness;
}

static const led_hal_interface_t PORT_HAL_SW_PWM = {
    .init = stm32_soft_pwm_init,
    .set_brightness = stm32_soft_pwm_set_brightness
};

/* External timer handle for software PWM (configured in CubeMX) */
extern TIM_HandleTypeDef htim2;

void Port_LED_Hardware_Init(drv_led_t *led_array)
{
    if (led_array == NULL) return;

    /* Bind logical LED objects to physical port implementations */
    drv_led_init(&led_array[LED_ID_0], &PORT_HAL_SW_PWM, &ctx_led0);
    drv_led_init(&led_array[LED_ID_1], &PORT_HAL_SW_PWM, &ctx_led1);

    /* Start the timer interrupt for soft PWM driving */
    HAL_TIM_Base_Start_IT(&htim2);
}

/* STM32 HAL Timer Callback for Soft PWM Generation */
void Port_LED_Timer_Handler(void)
{

	hw_soft_pwm_ctx_t *sw_led_contexts[] = { &ctx_led0, &ctx_led1 };

	for (int i = 0; i < LED_ID_MAX; i++) {
		hw_soft_pwm_ctx_t *ctx = sw_led_contexts[i];

		ctx->soft_pwm_counter++;
		if (ctx->soft_pwm_counter >= 100) ctx->soft_pwm_counter = 0;

		if (ctx->set_pin_state) {
			/* Compare counter with target brightness to generate PWM signal */
			if (ctx->soft_pwm_counter < ctx->target_brightness) {
				ctx->set_pin_state(1);
			} else {
				ctx->set_pin_state(0);
			}
		}
	}

}
