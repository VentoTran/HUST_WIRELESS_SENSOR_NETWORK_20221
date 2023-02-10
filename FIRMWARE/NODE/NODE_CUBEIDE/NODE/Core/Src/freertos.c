/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include "sx1278.h"
#include "common.h"
#include "timers.h"
#include "event_groups.h"
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
static const char *TAG = "FREERTOS";
EventGroupHandle_t sx1278_evt_group;
extern sx1278_node_t sx1278_node;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	if(GPIO_Pin == GPIO_PIN_6)
//	{
//		xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO4_BIT, &xHigherPriorityTaskWoken);
//	}
//	else if(GPIO_Pin == GPIO_PIN_7)
//	{
//		xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO3_BIT, &xHigherPriorityTaskWoken);
//	}
	if(GPIO_Pin == GPIO_PIN_12)
	{
		xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO0_BIT, &xHigherPriorityTaskWoken);
	}
}

void sx1278_task(void *param)
{
    uint8_t data_send[128] = {0};
    uint8_t data_recv[128] = {0};
    float snr;
    int rssi;
    EventBits_t evt_bits;
//    sx1278_packet_t packet;
    sx1278_init();
	sx1278_evt_group = xEventGroupCreate();
	float temp_test = 16.8;
	float battery_test = 3.58;
	while(1)
	{
//	    int irq = sx1278_read_reg(REG_IRQ_FLAGS);
//	    sx1278_write_reg(REG_IRQ_FLAGS, irq);
//		xEventGroupClearBits(sx1278_evt_group, SX1278_DIO0_BIT);
		sx1278_start_recv_data();
		evt_bits = xEventGroupWaitBits(sx1278_evt_group, SX1278_DIO0_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
		if(evt_bits & SX1278_DIO0_BIT)
		{
			if(sx1278_recv_data(data_recv, &rssi, &snr, &sx1278_node) == SX1278_OK)
			{
				char data_log[100];
				char snr_arr[10];
				ftoa((double)snr, snr_arr, 2);
				sprintf(data_log, "Packet rssi: %d, snr: %s", rssi, snr_arr);
				LOG(TAG, data_log);
				ftoa((double)temp_test, sx1278_node.temp, 1);
				ftoa((double)battery_test, sx1278_node.battery, 2);
				listen_before_talk();
				send_respond(UPLINK_TX_RESPOND_OPCODE, sx1278_node, data_send);
			}
			else
			{

			}
		}
	}
}

void peripheral_task(void *param)
{
	while(1)
	{
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

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
  /* definition and creation of defaultTask */
//  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
//  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	xTaskCreate(&sx1278_task, "SX1278", 512, NULL, 10, NULL);
	xTaskCreate(&peripheral_task, "PERIPHERAL", 512, NULL, 9, NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
