/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    jpeg.c
  * @brief   This file provides code for the configuration
  *          of the JPEG instances.
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "jpeg.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

JPEG_HandleTypeDef hjpeg;
MDMA_HandleTypeDef hmdma_jpeg_infifo_nf;
MDMA_HandleTypeDef hmdma_jpeg_outfifo_ne;

/* JPEG init function */
void MX_JPEG_Init(void)
{

  /* USER CODE BEGIN JPEG_Init 0 */

  /* USER CODE END JPEG_Init 0 */

  /* USER CODE BEGIN JPEG_Init 1 */

  /* USER CODE END JPEG_Init 1 */
  hjpeg.Instance = JPEG;
  if (HAL_JPEG_Init(&hjpeg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN JPEG_Init 2 */

  /* USER CODE END JPEG_Init 2 */

}

void HAL_JPEG_MspInit(JPEG_HandleTypeDef* jpegHandle)
{

  if(jpegHandle->Instance==JPEG)
  {
  /* USER CODE BEGIN JPEG_MspInit 0 */

  /* USER CODE END JPEG_MspInit 0 */
    /* JPEG clock enable */
    __HAL_RCC_JPEG_CLK_ENABLE();

    /* JPEG MDMA Init */
    /* JPEG_INFIFO_NF Init */
    hmdma_jpeg_infifo_nf.Instance = MDMA_Channel0;
    hmdma_jpeg_infifo_nf.Init.Request = MDMA_REQUEST_JPEG_INFIFO_NF;
    hmdma_jpeg_infifo_nf.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_jpeg_infifo_nf.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_jpeg_infifo_nf.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_jpeg_infifo_nf.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_jpeg_infifo_nf.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma_jpeg_infifo_nf.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_jpeg_infifo_nf.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
    hmdma_jpeg_infifo_nf.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_jpeg_infifo_nf.Init.BufferTransferLength = 4;
    hmdma_jpeg_infifo_nf.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_jpeg_infifo_nf.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_jpeg_infifo_nf.Init.SourceBlockAddressOffset = 0;
    hmdma_jpeg_infifo_nf.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_jpeg_infifo_nf) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_MDMA_ConfigPostRequestMask(&hmdma_jpeg_infifo_nf, 0, 0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(jpegHandle,hdmain,hmdma_jpeg_infifo_nf);

    /* JPEG_OUTFIFO_NE Init */
    hmdma_jpeg_outfifo_ne.Instance = MDMA_Channel1;
    hmdma_jpeg_outfifo_ne.Init.Request = MDMA_REQUEST_JPEG_OUTFIFO_NE;
    hmdma_jpeg_outfifo_ne.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_jpeg_outfifo_ne.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_jpeg_outfifo_ne.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_jpeg_outfifo_ne.Init.SourceInc = MDMA_SRC_INC_DISABLE;
    hmdma_jpeg_outfifo_ne.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    hmdma_jpeg_outfifo_ne.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
    hmdma_jpeg_outfifo_ne.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_jpeg_outfifo_ne.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_jpeg_outfifo_ne.Init.BufferTransferLength = 4;
    hmdma_jpeg_outfifo_ne.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_jpeg_outfifo_ne.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_jpeg_outfifo_ne.Init.SourceBlockAddressOffset = 0;
    hmdma_jpeg_outfifo_ne.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_jpeg_outfifo_ne) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_MDMA_ConfigPostRequestMask(&hmdma_jpeg_outfifo_ne, 0, 0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(jpegHandle,hdmaout,hmdma_jpeg_outfifo_ne);

    /* JPEG interrupt Init */
    HAL_NVIC_SetPriority(JPEG_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(JPEG_IRQn);
  /* USER CODE BEGIN JPEG_MspInit 1 */

  /* USER CODE END JPEG_MspInit 1 */
  }
}

void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef* jpegHandle)
{

  if(jpegHandle->Instance==JPEG)
  {
  /* USER CODE BEGIN JPEG_MspDeInit 0 */

  /* USER CODE END JPEG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_JPEG_CLK_DISABLE();

    /* JPEG MDMA DeInit */
    HAL_MDMA_DeInit(jpegHandle->hdmain);
    HAL_MDMA_DeInit(jpegHandle->hdmaout);

    /* JPEG interrupt Deinit */
    HAL_NVIC_DisableIRQ(JPEG_IRQn);
  /* USER CODE BEGIN JPEG_MspDeInit 1 */

  /* USER CODE END JPEG_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
