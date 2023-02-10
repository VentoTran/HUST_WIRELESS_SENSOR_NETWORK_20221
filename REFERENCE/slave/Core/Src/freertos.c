/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "onewire.h"
#include "DS18B20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	NOT_MEASURING,
	START_MEASURING,
	STOP_MEASURING
} status_measure_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern float temperature;
status_measure_t status_measure = NOT_MEASURING;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void button_task(void *argument);
void sensor_task(void *argument);

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

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(button_task, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(sensor_task, NULL, &myTask03_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_button_task */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_button_task */
void button_task(void *argument)
{
  /* USER CODE BEGIN button_task */
  /* Infinite loop */
  for(;;)
  {
	  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == 0)
	  {
		  HAL_Delay(150);
		  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == 0)
		  {
			  // todo: Start measuring
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			  status_measure = START_MEASURING;
		  }
	  } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == 0)
	  {
		  HAL_Delay(150);
		  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == 0)
		  {
			  // todo: Stop measuring
			  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
			  status_measure = STOP_MEASURING;
		  }
	  } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == 0)
	  {
		  HAL_Delay(150);
		  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == 0)
		  {
			  // todo: Display voltage battery...
		  }
	  }

	  osDelay(1);
  }
  /* USER CODE END button_task */
}

/* USER CODE BEGIN Header_sensor_task */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sensor_task */
void sensor_task(void *argument)
{
  /* USER CODE BEGIN sensor_task */
  /* Infinite loop */
  for(;;)
  {
    switch (status_measure)
    {
      case START_MEASURING: {
        DS18B20_ReadAll();
        DS18B20_StartAll();
        for(int i = 0; i < DS18B20_Quantity(); i++)
        {
          if(DS18B20_GetTemperature(i, &temperature))
          {
            osDelay(2000);
          }
        }
        break;
      }
      default: {break;}
	  }
    osDelay(1);
  }
  /* USER CODE END sensor_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

