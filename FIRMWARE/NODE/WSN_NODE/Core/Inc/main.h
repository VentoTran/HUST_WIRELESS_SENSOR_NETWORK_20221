/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOC
#define VBAT_Pin GPIO_PIN_1
#define VBAT_GPIO_Port GPIOA
#define CS1_Pin GPIO_PIN_4
#define CS1_GPIO_Port GPIOA
#define LoRa_EXTI2_Pin GPIO_PIN_12
#define LoRa_EXTI2_GPIO_Port GPIOB
#define LoRa_EXTI2_EXTI_IRQn EXTI15_10_IRQn
#define CS2_Pin GPIO_PIN_8
#define CS2_GPIO_Port GPIOA
#define LoRa_RST_Pin GPIO_PIN_15
#define LoRa_RST_GPIO_Port GPIOA
#define BT0_Pin GPIO_PIN_3
#define BT0_GPIO_Port GPIOB
#define BT0_EXTI_IRQn EXTI3_IRQn
#define BT1_Pin GPIO_PIN_4
#define BT1_GPIO_Port GPIOB
#define BT1_EXTI_IRQn EXTI4_IRQn
#define BT2_Pin GPIO_PIN_5
#define BT2_GPIO_Port GPIOB
#define BT2_EXTI_IRQn EXTI9_5_IRQn
#define LoRa_EXTI0_Pin GPIO_PIN_6
#define LoRa_EXTI0_GPIO_Port GPIOB
#define LoRa_EXTI0_EXTI_IRQn EXTI9_5_IRQn
#define LoRa_EXTI1_Pin GPIO_PIN_7
#define LoRa_EXTI1_GPIO_Port GPIOB
#define LoRa_EXTI1_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
