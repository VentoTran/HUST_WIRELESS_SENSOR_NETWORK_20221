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

#include "sx1278.h"
#include "common.h"

#include "portmacro.h"
#include "event_groups.h"

#include "display.h"
#include "DS18B20.h"

#include "adc.h"
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

#define VREFINT             (1.21)
#define ADC_RESOLUTION      (4095.0)
#define RATIO               (1.3)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

float temperature = 0;
float battery = 0;

status_measure_t status_measure = NOT_MEASURING;

bool isButtonPress = false;

static const char *TAG = "FREERTOS";
EventGroupHandle_t sx1278_evt_group;
int node_id;


/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void sx1278_task(void *param)
{
  // uint8_t data_send[128] = {0};
  // uint8_t data_recv[128] = {0};
  // float snr;
  // int rssi;
  // EventBits_t evt_bits;
  // sx1278_packet_t packet;
  // sx1278_respond_t respond;
  // respond.node_id = node_id;
  // sx1278_init();
	// sx1278_evt_group = xEventGroupCreate();
	while (1)
	{
		// sx1278_start_recv_data();
		// evt_bits = xEventGroupWaitBits(sx1278_evt_group, SX1278_DIO0_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
		// if(evt_bits & SX1278_DIO0_BIT)
		// {
		// 	if(sx1278_recv_data(data_recv, &rssi, &snr, &packet) == SX1278_OK)
		// 	{
		// 		char data_log[100];
		// 		char snr_arr[10];
		// 		ftoa((double)snr, snr_arr, 2);
		// 		sprintf(data_log, "Packet rssi: %d, snr: %s", rssi, snr_arr);
		// 		LOG(TAG, data_log);
		// 		respond.gate_id = packet.gate_id;
		// 		respond.period.float_val = packet.period.float_val;
		// 		respond.threshold.float_val = packet.threshold.float_val;
		// 		respond.temp.float_val = (float)10.0;
		// 		respond.battery.float_val = (float)3.75;
		// 		listen_before_talk();
		// 		send_respond(UPLINK_TX_RESPOND_OPCODE, respond, data_send);
		// 	}
		// 	else
		// 	{
		// 	}
		// }
    vTaskDelay(100 / portTICK_RATE_MS);
	}
}

void peripheral_task(void *param)
{

  DS18B20_Init(DS18B20_Resolution_12bits);
  displayInit();

  uint32_t adc[22] = {0};
  HAL_ADC_Start_DMA(&hadc1, adc, 20);
  
  uint16_t ADC_VREF_mV = 3300;


  static uint8_t swtch = 0;

  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

  uint32_t timeKeeper0 = HAL_GetTick();
  uint32_t timeKeeper1 = HAL_GetTick();
  uint32_t timeKeeper2 = HAL_GetTick();

	while(1)
	{

    // if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT0_Pin) == 0)
    // {
    //   vTaskDelay(150 / portTICK_RATE_MS);
    //   if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT0_Pin) == 0)
    //   {
		// 	  // todo: Start measuring
    //     // HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    //     status_measure = START_MEASURING;
    //   }
    // }
    // else if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT1_Pin) == 0)
    // {
    //   vTaskDelay(150 / portTICK_RATE_MS);
    //   if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT1_Pin) == 0)
    //   {
		// 	  // todo: Stop measuring
    //     // HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    //     status_measure = STOP_MEASURING;
    //   }
    // }
    // else if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT2_Pin) == 0)
    // {
    //   vTaskDelay(150 / portTICK_RATE_MS);
    //   if (HAL_GPIO_ReadPin(BT0_GPIO_Port, BT2_Pin) == 0)
    //   {
		// 	  // todo: Display voltage battery...
    //   }
    // } 

    if ((HAL_GetTick() - timeKeeper0) >= 1000)
    {
      DS18B20_ReadAll();
      DS18B20_StartAll();
      DS18B20_GetTemperature(0, &temperature);
      timeKeeper0 = HAL_GetTick();
    }

    if (((HAL_GetTick() - timeKeeper1) >= 3000) && (swtch == 0))
    {
      if ((temperature <= 50) && (temperature >= 0))
        displayFloat(temperature);
      swtch = 1;
      timeKeeper1 = HAL_GetTick();
    }
    if (((HAL_GetTick() - timeKeeper1) >= 3000) && (swtch == 1))
    {
      if ((battery <= 4.5) && (battery >= 2.4))
        displayFloat(battery);
      swtch = 0;
      timeKeeper1 = HAL_GetTick();
    }

    // if ()

    adc[20] = 0;
    adc[21] = 0;
    for (uint8_t i = 0; i < 20; i += 2)
    {
      adc[20] += adc[i];
      adc[21] += adc[i+1];
    }
    adc[20] /= 10;
    adc[21] /= 10;

    ADC_VREF_mV = (uint16_t)(VREFINT * ADC_RESOLUTION * 1000 / adc[21]);
    battery = (float)((float)adc[20] * RATIO * ADC_VREF_mV / ADC_RESOLUTION) / 1000;

		// vTaskDelay(1 / portTICK_RATE_MS);
    HAL_Delay(1);

	}
}


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

  displayStop();

  HAL_SuspendTick();
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
  HAL_ResumeTick();
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
  //indicate waking up 
  for (uint8_t i = 0; i < 6; i++)
  {
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
  }
  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);

  displayResume();
}
/* USER CODE END PREPOSTSLEEP */

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
  // osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  // defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	// xTaskCreate(&sx1278_task, "SX1278", 512, NULL, 10, NULL);
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if ((HAL_GPIO_ReadPin(GPIOB, GPIO_Pin) == 0) && (isButtonPress == false))
  {
    isButtonPress = true;
    __HAL_TIM_SetCounter(&htim4, 0);
    HAL_TIM_Base_Start_IT(&htim4);
  }
  if ((HAL_GPIO_ReadPin(GPIOB, GPIO_Pin) == 1) && (isButtonPress == true))
  {
    if (__HAL_TIM_GetCounter(&htim4) <= 500)
    {

    }
  }

  if (GPIO_Pin == BT0_Pin)
  {

  }
  else if (GPIO_Pin == BT1_Pin)
  {

  }
  else if (GPIO_Pin == BT2_Pin)
  {
    
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  if (htim->Instance == TIM3)
  {
    displayDigit(digIndex++);
    if (digIndex >= 4)  digIndex = 0;
  }

  if (htim->Instance == TIM4)
  {

  }


  /* USER CODE END Callback 1 */
}

/* USER CODE END Application */

