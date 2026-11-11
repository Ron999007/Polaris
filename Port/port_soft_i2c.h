/**
 * @file port_soft_i2c.h
 * @brief Software I2C (Bit-banging) Port Layer for STM32
 */

#ifndef PORT_SOFT_I2C_H
#define PORT_SOFT_I2C_H

#include "stm32h7xx_hal.h"

/* --- Hardware Binding Macros --- */
#define I2C_SCL_PORT        GPIOH
#define I2C_SCL_PIN         GPIO_PIN_6

#define I2C_SDA_PORT        GPIOG
#define I2C_SDA_PIN         GPIO_PIN_7

/* --- GPIO Control Macros --- */
#define I2C_SCL_H()         HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_L()         HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)

#define I2C_SDA_H()         HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_L()         HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)

#define I2C_SDA_READ()      HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN)

/* --- API Prototypes --- */
void Port_SoftI2C_Init(void);
uint8_t Port_SoftI2C_Mem_Write(uint8_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len);
uint8_t Port_SoftI2C_Mem_Read(uint8_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len);

#endif /* PORT_SOFT_I2C_H */
