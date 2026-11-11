#ifndef DRV_W9825G6KH_H
#define DRV_W9825G6KH_H

#include "stdint.h"
#include "fmc.h"

/* SDRAM Base Address and Size (2x W9825G6KH 16-bit -> 32-bit Bus) */
#define DRV_W9825G6KH_BASE_ADDR             ((uint32_t)0xC0000000)
#define DRV_W9825G6KH_TOTAL_SIZE            ((uint32_t)0x04000000) /* 64MB */

/* SDRAM Mode Register Definitions */
#define DRV_W9825G6KH_MODEREG_BURST_LENGTH_1        ((uint16_t)0x0000)
#define DRV_W9825G6KH_MODEREG_BURST_TYPE_SEQ        ((uint16_t)0x0000)
#define DRV_W9825G6KH_MODEREG_CAS_LATENCY_2         ((uint16_t)0x0020)
#define DRV_W9825G6KH_MODEREG_CAS_LATENCY_3         ((uint16_t)0x0030) /* Match CubeMX CL=3 */
#define DRV_W9825G6KH_MODEREG_OP_MODE_STANDARD      ((uint16_t)0x0000)
#define DRV_W9825G6KH_MODEREG_WRITEBURST_MODE_SING  ((uint16_t)0x0200)

/* Status Return Codes */
typedef enum {
    DRV_SDRAM_OK = 0,
    DRV_SDRAM_ERROR = 1,
    DRV_SDRAM_TIMEOUT = 2
} drv_sdram_status_t;

/* API Prototypes */
drv_sdram_status_t drv_w9825g6kh_init(SDRAM_HandleTypeDef *hsdram);

#endif /* DRV_W9825G6KH_H */
