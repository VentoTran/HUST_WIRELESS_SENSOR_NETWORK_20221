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
#include "sx1278.h"
#include "common.h"
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
extern int node_id;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void sx1278_task(void *param)
{
    uint8_t data_send[128] = {0};
    uint8_t data_recv[128] = {0};
    float snr;
    int rssi;
    EventBits_t evt_bits;
    sx1278_packet_t packet;
    sx1278_respond_t respond;
    respond.node_id = node_id;
    sx1278_init();
	sx1278_evt_group = xEventGroupCreate();
	while(1)
	{
		sx1278_start_recv_data();
		evt_bits = xEventGroupWaitBits(sx1278_evt_group, SX1278_DIO0_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
		if(evt_bits & SX1278_DIO0_BIT)
		{
			if(sx1278_recv_data(data_recv, &rssi, &snr, &packet) == SX1278_OK)
			{
				char data_log[100];
				char snr_arr[10];
				ftoa((double)snr, snr_arr, 2);
				sprintf(data_log, "Packet rssi: %d, snr: %s", rssi, snr_arr);
				LOG(TAG, data_log);
				respond.gate_id = packet.gate_id;
				respond.period.float_val = packet.period.float_val;
				respond.threshold.float_val = packet.threshold.float_val;
				respond.temp.float_val = (float)10.0;
				respond.battery.float_val = (float)3.75;
				listen_before_talk();
				send_respond(UPLINK_TX_RESPOND_OPCODE, respond, data_send);
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
