/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_LED */
osThreadId_t Task_LEDHandle;
const osThreadAttr_t Task_LED_attributes = {
  .name = "Task_LED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Button */
osThreadId_t Task_ButtonHandle;
const osThreadAttr_t Task_Button_attributes = {
  .name = "Task_Button",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_SDRAM */
osThreadId_t Task_SDRAMHandle;
const osThreadAttr_t Task_SDRAM_attributes = {
  .name = "Task_SDRAM",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_SN_Touch */
osThreadId_t Task_SN_TouchHandle;
const osThreadAttr_t Task_SN_Touch_attributes = {
  .name = "Task_SN_Touch",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_FatFS */
osThreadId_t Task_FatFSHandle;
const osThreadAttr_t Task_FatFS_attributes = {
  .name = "Task_FatFS",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_CAN */
osThreadId_t Task_CANHandle;
const osThreadAttr_t Task_CAN_attributes = {
  .name = "Task_CAN",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SdramReadySem */
osSemaphoreId_t SdramReadySemHandle;
const osSemaphoreAttr_t SdramReadySem_attributes = {
  .name = "SdramReadySem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void vTask_LED(void *argument);
void vTask_Button(void *argument);
void vTask_SDRAM(void *argument);
void vTask_SN_Touch(void *argument);
void vTask_FatFS(void *argument);
void vTask_CAN(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of SdramReadySem */
  SdramReadySemHandle = osSemaphoreNew(1, 0, &SdramReadySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_LED */
  Task_LEDHandle = osThreadNew(vTask_LED, NULL, &Task_LED_attributes);

  /* creation of Task_Button */
  Task_ButtonHandle = osThreadNew(vTask_Button, NULL, &Task_Button_attributes);

  /* creation of Task_SDRAM */
  Task_SDRAMHandle = osThreadNew(vTask_SDRAM, NULL, &Task_SDRAM_attributes);

  /* creation of Task_SN_Touch */
  Task_SN_TouchHandle = osThreadNew(vTask_SN_Touch, NULL, &Task_SN_Touch_attributes);

  /* creation of Task_FatFS */
  Task_FatFSHandle = osThreadNew(vTask_FatFS, NULL, &Task_FatFS_attributes);

  /* creation of Task_CAN */
  Task_CANHandle = osThreadNew(vTask_CAN, NULL, &Task_CAN_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vTask_LED */
/**
  * @brief  Function implementing the Task_LED thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vTask_LED */
__weak void vTask_LED(void *argument)
{
  /* USER CODE BEGIN vTask_LED */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_LED */
}

/* USER CODE BEGIN Header_vTask_Button */
/**
* @brief Function implementing the Task_Button thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTask_Button */
__weak void vTask_Button(void *argument)
{
  /* USER CODE BEGIN vTask_Button */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_Button */
}

/* USER CODE BEGIN Header_vTask_SDRAM */
/**
* @brief Function implementing the Task_SDRAM thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTask_SDRAM */
__weak void vTask_SDRAM(void *argument)
{
  /* USER CODE BEGIN vTask_SDRAM */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_SDRAM */
}

/* USER CODE BEGIN Header_vTask_SN_Touch */
/**
* @brief Function implementing the Task_SN_Touch thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTask_SN_Touch */
__weak void vTask_SN_Touch(void *argument)
{
  /* USER CODE BEGIN vTask_SN_Touch */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_SN_Touch */
}

/* USER CODE BEGIN Header_vTask_FatFS */
/**
* @brief Function implementing the Task_FatFS thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTask_FatFS */
__weak void vTask_FatFS(void *argument)
{
  /* USER CODE BEGIN vTask_FatFS */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_FatFS */
}

/* USER CODE BEGIN Header_vTask_CAN */
/**
* @brief Function implementing the Task_CAN thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTask_CAN */
__weak void vTask_CAN(void *argument)
{
  /* USER CODE BEGIN vTask_CAN */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vTask_CAN */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

