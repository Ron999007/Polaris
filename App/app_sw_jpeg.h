/**
 * @file app_sw_jpeg.h
 * @brief Software JPEG Decoder Wrapper for Polaris OS (Double Buffering Supported)
 */

#ifndef APP_SW_JPEG_H
#define APP_SW_JPEG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/**
 * @brief Target memory address for JPEG decoding.
 * Defaults to the LCD Framebuffer (0xC0000000).
 * Can be changed to a Back-Buffer address for transition effects.
 */
extern uint32_t jpeg_decode_target;

/**
 * @brief  Decodes and displays a JPEG file with AUTO-SCALING, CENTER, and ROTATE.
 * @param  filename: The path to the JPEG file (e.g., "0:/Picture/test.jpg").
 * @param  x_pos: X-coordinate bounding box start.
 * @param  y_pos: Y-coordinate bounding box start.
 * @retval 0 on success, non-zero on failure.
 */
uint8_t APP_SW_JPEG_Draw(const char *filename, uint16_t x_pos, uint16_t y_pos);

#ifdef __cplusplus
}
#endif

#endif /* APP_SW_JPEG_H */
