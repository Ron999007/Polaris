#include "drv_w9825g6kh.h"
#include "cmsis_os.h" /* For FreeRTOS osDelay API */

/* Timeout definition for FMC commands */
#define DRV_W9825G6KH_TIMEOUT ((uint32_t)0xFFFF)

/**
 * @brief  Initializes the W9825G6KH SDRAM hardware sequence.
 * @param  hsdram: Pointer to a SDRAM_HandleTypeDef structure
 * @retval DRV_SDRAM_OK on success, error code otherwise
 */
drv_sdram_status_t drv_w9825g6kh_init(SDRAM_HandleTypeDef *hsdram)
{
    FMC_SDRAM_CommandTypeDef command;

    if (hsdram == NULL) {
        return DRV_SDRAM_ERROR;
    }

    /* 1. Send Clock Enable command */
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(hsdram, &command, DRV_W9825G6KH_TIMEOUT) != HAL_OK) {
        return DRV_SDRAM_ERROR;
    }

    /* 2. Delay at least 100us. Yields to OS and waits ~1ms */
    //osDelay(5);
    for(volatile uint32_t i = 0; i < 0x3FFFF; i++)
	{
		/* Software delay loop */
	}


    /* 3. Send PALL (Precharge All) command */
    command.CommandMode = FMC_SDRAM_CMD_PALL;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(hsdram, &command, DRV_W9825G6KH_TIMEOUT) != HAL_OK) {
        return DRV_SDRAM_ERROR;
    }

    /* 4. Send Auto Refresh command (8 consecutive cycles required) */
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.AutoRefreshNumber = 8;
    command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(hsdram, &command, DRV_W9825G6KH_TIMEOUT) != HAL_OK) {
        return DRV_SDRAM_ERROR;
    }

    /* 5. Program the Load Mode Register (CL=3) */
    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = (uint32_t)(DRV_W9825G6KH_MODEREG_BURST_LENGTH_1 |
                                                DRV_W9825G6KH_MODEREG_BURST_TYPE_SEQ |
                                                DRV_W9825G6KH_MODEREG_CAS_LATENCY_3 |
                                                DRV_W9825G6KH_MODEREG_OP_MODE_STANDARD |
                                                DRV_W9825G6KH_MODEREG_WRITEBURST_MODE_SING);
    if (HAL_SDRAM_SendCommand(hsdram, &command, DRV_W9825G6KH_TIMEOUT) != HAL_OK) {
        return DRV_SDRAM_ERROR;
    }

    /* 6. Set the refresh rate counter (For 180MHz FMC clock -> 90MHz SDRAM) */
	/* Rate = (64ms / 8192 rows) * 90MHz - 20 = 683 */
	if (HAL_SDRAM_ProgramRefreshRate(hsdram, 683) != HAL_OK) {
		return DRV_SDRAM_ERROR;
	}

    return DRV_SDRAM_OK;
}
