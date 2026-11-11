/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   FatFs diskio driver with Dual Drive Routing (CubeMX Compatible)
  ******************************************************************************
  */
#include <stdio.h>
#include "drv_snor_flash.h"
#include "drv_sdhc.h"
#include "cmsis_os.h"

/* ==================================================================== */
/* Debug Configuration                                                  */
/* ==================================================================== */
#define ENABLE_DISKIO_DEBUG 1

#if ENABLE_DISKIO_DEBUG
    #define DISKIO_PRINTF(...) printf(__VA_ARGS__)
#else
    #define DISKIO_PRINTF(...)
#endif

/* ==================================================================== */
/* Drive Routing Definitions                                            */
/* ==================================================================== */
#define DRIVE_SPI_NOR  0
#define DRIVE_SDHC     1

 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* Independent status flags for each drive */
static volatile DSTATUS Stat_SPI = STA_NOINIT;
static volatile DSTATUS Stat_SDHC = STA_NOINIT;

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    return USER_status(pdrv);
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    if (pdrv == DRIVE_SPI_NOR) {
        uint32_t u32_snor_id = 0;
        if (drv_snor_read_id(&u32_snor_id) == SNOR_OK) {
            if (u32_snor_id == 0xEF4019 || u32_snor_id == 0xEF7019) {
                Stat_SPI &= ~STA_NOINIT;
            } else {
                Stat_SPI |= STA_NOINIT;
            }
        } else {
            Stat_SPI |= STA_NOINIT;
        }
        return Stat_SPI;
    }
    else if (pdrv == DRIVE_SDHC) {
        SD_HandleTypeDef *hsd = drv_sdhc_get_handle();
        if (hsd == NULL) return STA_NOINIT;

        /* Using %lu for card state to match HAL types on H7 */
        HAL_SD_CardStateTypedef card_state = HAL_SD_GetCardState(hsd);

        if (card_state == HAL_SD_CARD_TRANSFER) {
            Stat_SDHC &= ~STA_NOINIT;
        } else {
            DISKIO_PRINTF("[DiskIO] SDHC Not in Transfer State: %lu\r\n", (uint32_t)card_state);
            Stat_SDHC |= STA_NOINIT;
        }
        return Stat_SDHC;
    }

    return STA_NOINIT;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
    if (pdrv == DRIVE_SPI_NOR) {
        uint32_t address = sector * W25Q256_SECTOR_SIZE;
        uint32_t size = count * W25Q256_SECTOR_SIZE;

        if (drv_snor_read(address, buff, size) == SNOR_OK) {
            return RES_OK;
        }
        return RES_ERROR;
    }
    else if (pdrv == DRIVE_SDHC) {
        SD_HandleTypeDef *hsd = drv_sdhc_get_handle();
        if (hsd == NULL) return RES_ERROR;

        /* SDHC internal IDMA requires 4-byte aligned buffer */
        if (((uint32_t)buff & 0x03) != 0) {
            DISKIO_PRINTF("[DiskIO] [ERROR] Unaligned SDHC Read Buffer!\r\n");
            return RES_ERROR;
        }

        if (HAL_SD_ReadBlocks(hsd, buff, sector, count, 10000) == HAL_OK) {
            while(HAL_SD_GetCardState(hsd) != HAL_SD_CARD_TRANSFER) {
                osDelay(1);
            }
            return RES_OK;
        } else {
            DISKIO_PRINTF("[DiskIO] [ERROR] SDHC Read Failed (LBA: %lu)\r\n", sector);
            return RES_ERROR;
        }
    }

    return RES_PARERR;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
    if (pdrv == DRIVE_SPI_NOR) {
        uint32_t address;
        uint32_t offset = 0;
        uint32_t pages_per_sector = W25Q256_SECTOR_SIZE / W25Q256_PAGE_SIZE;

        for (UINT i = 0; i < count; i++) {
            address = (sector + i) * W25Q256_SECTOR_SIZE;
            if (drv_snor_erase_sector(address) != SNOR_OK) return RES_ERROR;

            for (uint32_t page = 0; page < pages_per_sector; page++) {
                uint32_t write_addr = address + (page * W25Q256_PAGE_SIZE);
                if (drv_snor_write_page(write_addr, (uint8_t *)(buff + offset), W25Q256_PAGE_SIZE) != SNOR_OK) {
                    return RES_ERROR;
                }
                offset += W25Q256_PAGE_SIZE;
            }
        }
        return RES_OK;
    }
    else if (pdrv == DRIVE_SDHC) {
        SD_HandleTypeDef *hsd = drv_sdhc_get_handle();
        if (hsd == NULL) return RES_ERROR;

        if (((uint32_t)buff & 0x03) != 0) {
            DISKIO_PRINTF("[DiskIO] [ERROR] Unaligned SDHC Write Buffer!\r\n");
            return RES_ERROR;
        }

        if (HAL_SD_WriteBlocks(hsd, (uint8_t*)buff, sector, count, 10000) == HAL_OK) {
            while(HAL_SD_GetCardState(hsd) != HAL_SD_CARD_TRANSFER) {
                osDelay(1);
            }
            return RES_OK;
        } else {
            DISKIO_PRINTF("[DiskIO] [ERROR] SDHC Write Failed (LBA: %lu)\r\n", sector);
            return RES_ERROR;
        }
    }

    return RES_PARERR;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;

    if (pdrv == DRIVE_SPI_NOR) {
        switch (cmd) {
            case CTRL_SYNC:
                res = RES_OK;
                break;
            case GET_SECTOR_COUNT:
                *(DWORD*)buff = W25Q256_FLASH_SIZE / W25Q256_SECTOR_SIZE;
                res = RES_OK;
                break;
            case GET_SECTOR_SIZE:
                *(WORD*)buff = W25Q256_SECTOR_SIZE;
                res = RES_OK;
                break;
            case GET_BLOCK_SIZE:
                *(DWORD*)buff = W25Q256_BLOCK64_SIZE / W25Q256_SECTOR_SIZE;
                res = RES_OK;
                break;
            default:
                res = RES_PARERR;
                break;
        }
    }
    else if (pdrv == DRIVE_SDHC) {
        SD_HandleTypeDef *hsd = drv_sdhc_get_handle();
        HAL_SD_CardInfoTypeDef CardInfo;
        if (hsd == NULL) return RES_ERROR;

        switch (cmd) {
            case CTRL_SYNC:
                res = RES_OK;
                break;
            case GET_SECTOR_COUNT:
                HAL_SD_GetCardInfo(hsd, &CardInfo);
                *(DWORD*)buff = CardInfo.LogBlockNbr;
                res = RES_OK;
                break;
            case GET_SECTOR_SIZE:
                *(WORD*)buff = 512;
                res = RES_OK;
                break;
            case GET_BLOCK_SIZE:
                *(DWORD*)buff = 1;
                res = RES_OK;
                break;
            default:
                res = RES_PARERR;
                break;
        }
    }

    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

