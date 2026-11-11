/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "rtc.h"
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */
uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	DWORD fattime = 0;

	/* Get the RTC current Time and Date */
	/* Note: Must call HAL_RTC_GetTime first, then HAL_RTC_GetDate to unlock the values */
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	/* Pack date and time into a DWORD variable */
	/* bit31:25 Year origin from the 1980 (0..127, e.g. 37 for 2017) */
	/* bit24:21 Month (1..12) */
	/* bit20:16 Day of the month (1..31) */
	/* bit15:11 Hour (0..23) */
	/* bit10:5  Minute (0..59) */
	/* bit4:0   Second / 2 (0..29, e.g. 25 for 50s) */

	/* The RTC year is 0-99 (meaning 2000-2099). FatFS year starts from 1980. */
	/* So FatFS year = (RTC year + 2000) - 1980 = RTC year + 20 */
	fattime = ((DWORD)(sDate.Year + 20) << 25)
		  | ((DWORD)sDate.Month << 21)
		  | ((DWORD)sDate.Date  << 16)
		  | ((DWORD)sTime.Hours << 11)
		  | ((DWORD)sTime.Minutes << 5)
		  | ((DWORD)sTime.Seconds / 2);

	return fattime;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
