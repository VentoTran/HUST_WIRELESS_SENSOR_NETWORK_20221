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

#include "time.h"
#include "timers.h"
#include "portmacro.h"
#include "event_groups.h"

#include "display.h"
#include "DS18B20.h"
#include "sx1278.h"
#include "common.h"

#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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


uint32_t adc[22] = {0};

bool isButtonPress = false;
bool isSleep = false;

uint8_t btPressed = 0xFF;

static const char *TAG = "FREERTOS";
EventGroupHandle_t sx1278_evt_group;
extern sx1278_node_t sx1278_node;


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
  sx1278_init();
	sx1278_evt_group = xEventGroupCreate();
	while(1)
	{
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
				listen_before_talk();
				send_respond(UPLINK_TX_RESPOND_OPCODE, sx1278_node, data_send);
			}
			else
			{
        //Do nothing
			}
		}
	}
}

void peripheral_task(void *param)
{

  DS18B20_Init(DS18B20_Resolution_12bits);
  displayInit();

  HAL_ADC_Start_DMA(&hadc1, adc, 20);
  
  static uint8_t sw = 0;
  uint16_t ADC_VREF_mV = 3300;

  HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

  float temperature = 0;
  float battery = 0;

  HAL_TIM_Base_Start_IT(&htim4);

  uint32_t timeKeeper0 = HAL_GetTick();
  uint32_t timeKeeper1 = HAL_GetTick() - 2000;
  uint32_t timeKeeper2 = HAL_GetTick();
  uint32_t timeKeeper3 = HAL_GetTick();

	while(1)
	{
    timeKeeper0 = HAL_GetTick();
    timeKeeper1 = HAL_GetTick() - 2000;
    timeKeeper2 = HAL_GetTick();
    timeKeeper3 = HAL_GetTick();
    while ((HAL_GetTick() - timeKeeper2) <= 6000)
    {
      if ((HAL_GetTick() - timeKeeper0) >= 1000)
      {
        DS18B20_Read(0, &temperature);
        DS18B20_Start(0);
        timeKeeper0 = HAL_GetTick();
      }

      if ((btPressed == 0) && (temperature <= 100) && (temperature >= -10))
      {
        displayFloat(temperature);
      }
      else if ((btPressed == 1) && (battery <= 4.5) && (battery >= 2.4))
      {
        displayFloat(battery);
      }
      else if (btPressed == 2)
      {
        displayInt(sx1278_node.node_id);
      }
      else if (btPressed == 0xFF)
      {
        if ((HAL_GetTick() - timeKeeper1) >= 2000)
        {
          if (sw == 0)
          {
            displayInt(sx1278_node.node_id);
            sw = 1;
          }
          else if (sw == 1)
          {
            displayFloat(temperature);
            sw = 2;
          }
          else if (sw == 2)
          {
            displayFloat(battery);
            sw = 0;
          }
          timeKeeper1 = HAL_GetTick();
        }
      }

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
      battery = (float)(((float)adc[20] * RATIO * ADC_VREF_mV / ADC_RESOLUTION) / 1000) - 0.1;
      ftoa((double)temperature, sx1278_node.temp, 2);
      ftoa((double)battery, sx1278_node.battery, 2);
      HAL_Delay(100);
    }
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    displayInt(0);
    btPressed = 0xFF;
    sw = 0;
    isButtonPress = false;
    isSleep = true;
		vTaskDelay(1 / portTICK_RATE_MS);
	}
}

void vTimerCallback(TimerHandle_t xTimer)
{
  
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
    task. It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()). If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */

  if (isSleep == true)
  {
    HAL_ADC_Stop_DMA(&hadc1);
    displayStop();

    HAL_SuspendTick();

    HAL_PWR_EnterSLEEPMode(1, PWR_SLEEPENTRY_WFI);
  }


}
/* USER CODE END 2 */

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
  // xTimerCreate("stimer0", 2000/portTICK_RATE_MS, pdTRUE, NULL, vTimerCallback);
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
  xTaskCreate(&sx1278_task, "SX1278", 256, NULL, 9, NULL);
  xTaskCreate(&peripheral_task, "PERIPHERAL", 256, NULL, 10, NULL);
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
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if ((HAL_GPIO_ReadPin(GPIOB, GPIO_Pin) == 0) && (isButtonPress == false) && (GPIO_Pin != GPIO_PIN_12))
  {
    isButtonPress = true;
    if (GPIO_Pin == BT0_Pin)
    {
      btPressed = 0;
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    }
    else if (GPIO_Pin == BT1_Pin)
    {
      btPressed = 1;
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    }
    else if (GPIO_Pin == BT2_Pin)
    {
      btPressed = 2;
      HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
    }
    displayResume();
  }

  if((HAL_GPIO_ReadPin(GPIOB, GPIO_Pin) == 1) && (GPIO_Pin == GPIO_PIN_12))
	{
		xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO0_BIT, &xHigherPriorityTaskWoken);
	}

  if (isSleep == true)
  {
    HAL_ResumeTick();
    HAL_ADC_Start_DMA(&hadc1, adc, 20);
    // displayResume();
    
    isSleep = false;
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
    if (isSleep == true)
    {
      HAL_ResumeTick();
      HAL_ADC_Start_DMA(&hadc1, adc, 20);
      // displayResume();
      // HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
      // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
      isSleep = false;
    }
  }


  /* USER CODE END Callback 1 */
}

/* USER CODE END Application */

