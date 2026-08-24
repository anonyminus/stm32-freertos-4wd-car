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
#include "auto_drive.h"
#include "button.h"
#include "manual_drive.h"
#include "ultrasonic.h"
#include "wifi.h"
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

osThreadId_t ultrasonicTaskHandle;
osThreadId_t autoDriveTaskHandle;
osThreadId_t buttonTaskHandle;
osThreadId_t manualDriveTaskHandle;
osThreadId_t wifiTaskHandle;

static const osThreadAttr_t ultrasonicTaskAttributes = {
    .name = "UltrasonicTask",
    .stack_size = 512U * 4U,
    .priority = (osPriority_t)osPriorityNormal
};

static const osThreadAttr_t autoDriveTaskAttributes = {
    .name = "AutoDriveTask",
    .stack_size = 512U * 4U,
    .priority = (osPriority_t)osPriorityAboveNormal
};

static const osThreadAttr_t buttonTaskAttributes = {
    .name = "ButtonTask",
    .stack_size = 128U * 4U,
    .priority = (osPriority_t)osPriorityHigh
};

static const osThreadAttr_t manualDriveTaskAttributes = {
    .name = "ManualDriveTask",
    .stack_size = 384U * 4U,
    .priority = (osPriority_t)osPriorityAboveNormal
};

static const osThreadAttr_t wifiTaskAttributes = {
    .name = "WifiTask",
    .stack_size = 384U * 4U,
    .priority = (osPriority_t)osPriorityHigh
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(
    xTaskHandle xTask,
    signed char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();

    for (;;)
    {
    }
}
/* USER CODE END 5 */

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
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  ultrasonicTaskHandle = osThreadNew(
      UltrasonicTask,
      NULL,
      &ultrasonicTaskAttributes
  );

  autoDriveTaskHandle = osThreadNew(
      AutoDriveTask,
      NULL,
      &autoDriveTaskAttributes
  );

  buttonTaskHandle = osThreadNew(
      ButtonTask,
      NULL,
      &buttonTaskAttributes
  );

  manualDriveTaskHandle = osThreadNew(
      ManualDriveTask,
      NULL,
      &manualDriveTaskAttributes
  );

  wifiTaskHandle = osThreadNew(
      WifiTask,
      NULL,
      &wifiTaskAttributes
  );

  if ((defaultTaskHandle == NULL) ||
      (ultrasonicTaskHandle == NULL) ||
      (autoDriveTaskHandle == NULL) ||
      (buttonTaskHandle == NULL) ||
      (manualDriveTaskHandle == NULL) ||
      (wifiTaskHandle == NULL))
  {
      Error_Handler();
  }

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

    (void)argument;

    for (;;)
    {
        osDelay(1000U);
    }

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

