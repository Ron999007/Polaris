/**
 * @file app_hw_jpeg.h
 * @brief Hardware Accelerated JPEG Decoder for STM32H7 (JPEG Codec + DMA2D)
 */

#ifndef APP_HW_JPEG_H
#define APP_HW_JPEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief  Decodes and displays a JPEG file from the SD Card directly to the LCD.
 * @param  filename: The path to the JPEG file (e.g., "0:/photo.jpg")
 * @param  x_pos: X-coordinate on the LCD
 * @param  y_pos: Y-coordinate on the LCD
 * @retval 0 on success, 1 on failure
 */
uint8_t APP_JPEG_Draw(const char *filename, uint16_t x_pos, uint16_t y_pos);

#ifdef __cplusplus
}
#endif

#endif /* APP_HW_JPEG_H */
