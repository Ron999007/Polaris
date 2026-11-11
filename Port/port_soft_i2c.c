/**
 * @file port_soft_i2c.c
 * @brief Software I2C (Bit-banging) Implementation
 */

#include "port_soft_i2c.h"

/**
 * @brief Microsecond delay for I2C timing
 * @note Tuned for 360MHz SYSCLK to provide a safe delay for I2C protocol
 */
static void I2C_Delay(void)
{
    /* Loop count adjusted for 360MHz to ensure stable communication */
    for (volatile uint32_t i = 0; i < 45; i++) {}
}

void Port_SoftI2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. Enable Clocks for GPIOH (SCL) and GPIOG (SDA) */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* 2. Configure SCL (PH6) as Open-Drain output with Pull-Up */
    GPIO_InitStruct.Pin = I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C_SCL_PORT, &GPIO_InitStruct);

    /* 3. Configure SDA (PG7) as Open-Drain output with Pull-Up */
    GPIO_InitStruct.Pin = I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_InitStruct);

    /* 4. Set buses high (Idle state) */
    I2C_SCL_H();
    I2C_SDA_H();
}

static void I2C_Start(void)
{
    I2C_SDA_H();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_L();
}

static void I2C_Stop(void)
{
    I2C_SCL_L();
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_H();
    I2C_SDA_H();
    I2C_Delay();
}

static uint8_t I2C_WaitAck(void)
{
    uint8_t err_time = 0;
    I2C_SDA_H();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();

    while (I2C_SDA_READ())
    {
        err_time++;
        if (err_time > 250)
        {
            I2C_Stop();
            return 1;
        }
    }
    I2C_SCL_L();
    return 0;
}

static void I2C_Ack(void)
{
    I2C_SCL_L();
    I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
}

static void I2C_NAck(void)
{
    I2C_SCL_L();
    I2C_SDA_H();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
}

static void I2C_SendByte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if ((byte & 0x80) >> 7)
            I2C_SDA_H();
        else
            I2C_SDA_L();

        byte <<= 1;
        I2C_Delay();
        I2C_SCL_H();
        I2C_Delay();
        I2C_SCL_L();
        I2C_Delay();
    }
}

static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t i, receive = 0;
    I2C_SDA_H();

    for (i = 0; i < 8; i++)
    {
        I2C_SCL_L();
        I2C_Delay();
        I2C_SCL_H();
        receive <<= 1;
        if (I2C_SDA_READ()) receive++;
        I2C_Delay();
    }

    if (!ack) I2C_NAck();
    else I2C_Ack();

    return receive;
}

/* --- Exported Memory Read/Write APIs --- */

uint8_t Port_SoftI2C_Mem_Write(uint8_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    I2C_Start();
    I2C_SendByte(dev_addr);
    if (I2C_WaitAck()) return 1;

    I2C_SendByte((uint8_t)(reg_addr >> 8));
    if (I2C_WaitAck()) return 1;
    I2C_SendByte((uint8_t)(reg_addr & 0xFF));
    if (I2C_WaitAck()) return 1;

    for (uint16_t i = 0; i < len; i++)
    {
        I2C_SendByte(data[i]);
        if (I2C_WaitAck()) return 1;
    }
    I2C_Stop();
    return 0;
}

uint8_t Port_SoftI2C_Mem_Read(uint8_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len)
{
    I2C_Start();
    I2C_SendByte(dev_addr);
    if (I2C_WaitAck()) return 1;

    I2C_SendByte((uint8_t)(reg_addr >> 8));
    if (I2C_WaitAck()) return 1;
    I2C_SendByte((uint8_t)(reg_addr & 0xFF));
    if (I2C_WaitAck()) return 1;

    I2C_Start();
    I2C_SendByte(dev_addr | 0x01);
    if (I2C_WaitAck()) return 1;

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = I2C_ReadByte(i == (len - 1) ? 0 : 1);
    }
    I2C_Stop();
    return 0;
}
