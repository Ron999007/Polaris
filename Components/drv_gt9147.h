/**
 * @file drv_gt9147.h
 * @brief Driver for GT9147 Capacitive Touch Controller (I2C Interface)
 * @project Polaris
 */

#ifndef DRV_GT9147_H
#define DRV_GT9147_H

#include <stdint.h>

/* GT9147 I2C Device Address (0x14 -> 7-bit, 0x28 -> 8-bit for HAL) */
#define GT9147_I2C_ADDR         0x28

/* GT9147 Register Definitions */
#define GT9147_REG_COMMAND      0x8040
#define GT9147_REG_CONFIG       0x8047
#define GT9147_REG_PRODUCT_ID   0x8140
#define GT9147_REG_STATUS       0x814E
#define GT9147_REG_POINT1_X_L   0x8150

/**
 * @brief Structure to store touch point information
 */
typedef struct {
    uint16_t x;             /* X coordinate (0-799 for 800x480 display) */
    uint16_t y;             /* Y coordinate (0-479 for 800x480 display) */
    uint8_t  is_pressed;    /* 1 if touch is detected, 0 otherwise */
} drv_gt9147_point_t;

/**
 * @brief Driver status enumeration
 */
typedef enum {
    DRV_GT9147_OK = 0,      /* Operation successful */
    DRV_GT9147_ERROR = 1,   /* I2C or hardware failure */
    DRV_GT9147_NO_TOUCH = 2 /* No touch detected during scan */
} drv_gt9147_status_t;

/* --- API Prototypes --- */

/**
 * @brief Initializes the GT9147 touch controller.
 * This includes GPIO initialization, hardware reset sequence to set
 * the I2C address, and software reset via I2C commands.
 * * @return DRV_GT9147_OK if initialization and ID verification are successful.
 */
drv_gt9147_status_t drv_gt9147_init(void);

/**
 * @brief Reads the current touch status and coordinates from the GT9147.
 * This function is non-blocking and should be called periodically (e.g., every 20ms).
 * * @param point Pointer to a drv_gt9147_point_t structure to store the result.
 * @return DRV_GT9147_OK if touch data is successfully read, or DRV_GT9147_NO_TOUCH.
 */
drv_gt9147_status_t drv_gt9147_read_touch(drv_gt9147_point_t *point);

#endif /* DRV_GT9147_H */
