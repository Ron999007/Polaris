/**
 * @file app_sw_jpeg.c
 * @brief Software JPEG Decoder Implementation (Double Buffering Ready)
 */

#include "app_sw_jpeg.h"
#include "tjpgd.h"
#include "ff.h"
#include <stdio.h>

/* Display Definitions */
#define PHYSICAL_W      800
#define PHYSICAL_H      480
#define LOGICAL_W       480
#define LOGICAL_H       800
#define LCD_DIR_MODE    0

/* 🌟 Global target address. Defaults to Front Buffer */
uint32_t jpeg_decode_target = 0xC0000000UL;

#define TJPGD_WORKSPACE_SIZE 3500
ALIGN_32BYTES(static uint8_t JpegWorkSpace[TJPGD_WORKSPACE_SIZE]);

typedef struct {
    FIL *file;
    uint16_t start_x;
    uint16_t start_y;
    uint8_t auto_rotate;
    uint16_t scaled_w;
    uint16_t scaled_h;
} JPEGSESSION;

static size_t in_func(JDEC* jd, uint8_t* buff, size_t nbyte) {
    JPEGSESSION *dev = (JPEGSESSION*)jd->device;
    UINT bytes_read;
    if (buff) {
        f_read(dev->file, buff, nbyte, &bytes_read);
        return (size_t)bytes_read;
    } else {
        f_lseek(dev->file, f_tell(dev->file) + nbyte);
        return nbyte;
    }
}

static int out_func(JDEC* jd, void* bitmap, JRECT* rect) {
    JPEGSESSION *dev = (JPEGSESSION*)jd->device;
    uint16_t *bmp = (uint16_t*)bitmap;

    for (uint16_t y = rect->top; y <= rect->bottom; y++) {
        for (uint16_t x = rect->left; x <= rect->right; x++) {
            uint16_t color = *bmp++;
            uint16_t draw_x, draw_y;

            if (dev->auto_rotate) {
                draw_x = dev->start_x + (dev->scaled_h - 1 - y);
                draw_y = dev->start_y + x;
            } else {
                draw_x = dev->start_x + x;
                draw_y = dev->start_y + y;
            }

            if (draw_x >= LOGICAL_W || draw_y >= LOGICAL_H) continue;

            uint16_t px, py;
#if LCD_DIR_MODE == 0
            px = draw_y;
            py = (PHYSICAL_H - 1) - draw_x;
#else
            px = (PHYSICAL_W - 1) - draw_y;
            py = draw_x;
#endif
            /* 🌟 Write to the dynamic target buffer instead of hardcoded LCD */
            ((uint16_t *)jpeg_decode_target)[py * PHYSICAL_W + px] = color;
        }
    }
    return 1;
}

uint8_t APP_SW_JPEG_Draw(const char *filename, uint16_t x_pos, uint16_t y_pos) {
    FRESULT res;
    JRESULT jres;
    JDEC jdec;
    JPEGSESSION session;
    FIL file;

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        printf("[JPEG SW] File open failed: %d\r\n", res);
        return 1;
    }

    session.file = &file;
    jres = jd_prepare(&jdec, in_func, JpegWorkSpace, TJPGD_WORKSPACE_SIZE, &session);
    if (jres != JDR_OK) {
        printf("[JPEG SW] Format error: %d\r\n", jres);
        f_close(&file);
        return 1;
    }

    uint8_t is_img_landscape = (jdec.width > jdec.height) ? 1 : 0;
    uint8_t is_lcd_portrait = (LOGICAL_H > LOGICAL_W) ? 1 : 0;

    session.auto_rotate = (is_img_landscape && is_lcd_portrait) ? 1 : 0;

    uint16_t visual_w = session.auto_rotate ? jdec.height : jdec.width;
    uint16_t visual_h = session.auto_rotate ? jdec.width : jdec.height;

    uint8_t scale = 0;
    while (((visual_w >> scale) > LOGICAL_W) || ((visual_h >> scale) > LOGICAL_H)) {
        scale++;
        if (scale >= 3) break;
    }

    session.scaled_w = jdec.width >> scale;
    session.scaled_h = jdec.height >> scale;

    uint16_t final_w = visual_w >> scale;
    uint16_t final_h = visual_h >> scale;

    session.start_x = x_pos + ((final_w < LOGICAL_W) ? (LOGICAL_W - final_w) / 2 : 0);
    session.start_y = y_pos + ((final_h < LOGICAL_H) ? (LOGICAL_H - final_h) / 2 : 0);

    jres = jd_decomp(&jdec, out_func, scale);

    if (jres == JDR_OK) {
        /* 🌟 Clean the cache for the specific target buffer */
        SCB_CleanDCache_by_Addr((uint32_t *)jpeg_decode_target, PHYSICAL_W * PHYSICAL_H * 2);
    }
    f_close(&file);
    return (jres == JDR_OK) ? 0 : 1;
}
