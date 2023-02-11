/**
 * @file display.c
 * @author Vento (zseefvhu12345@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2023-02-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "display.h"

static const uint8_t NumberTable[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
static const uint8_t DP = 0x80;
static uint8_t digBuffer[4] = {0};
static uint8_t posPoint = 5;

uint8_t digIndex = 0;

void displayInt(int i)
{
	memset(digBuffer, '\0', sizeof(digBuffer));

	digBuffer[3] = (uint8_t)(i / 1000);
	digBuffer[2] = (uint8_t)((i % 1000) / 100);
	digBuffer[1] = (uint8_t)((i % 100) / 10);
	digBuffer[0] = (uint8_t)(i % 10);

	posPoint = 5;
}

void displayFloat(float f)
{
	memset(digBuffer, '\0', sizeof(digBuffer));
	int n = (int) f;
	int d = 0;
	if (n >= 10)
	{
		d = (int)((float)(f - n) * 100);
		digBuffer[3] = (uint8_t)(n / 10);
		digBuffer[2] = (uint8_t)(n % 10);
		digBuffer[1] = (uint8_t)(d / 10);
		digBuffer[0] = (uint8_t)(d % 10);
		posPoint = 2;
	}
	else
	{
		d = (int)((float)(f - n) * 1000);
		digBuffer[3] = (uint8_t)(n);
		digBuffer[2] = (uint8_t)(d / 100);
		digBuffer[1] = (uint8_t)((d % 100) / 10);
		digBuffer[0] = (uint8_t)(d % 10);
		posPoint = 3;
	}
}

void displayInit(void)
{
    HC595_Init();
    HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
    memset(digBuffer, '\0', sizeof(digBuffer));

    HAL_TIM_Base_Start_IT(&htim3);
}

void displayDigit(uint8_t dig)
{
	switch (dig)
	{
		case 0: {
			HC595_SendByte(NumberTable[digBuffer[0]]);
			HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_SET);
			// HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
			break;
		}
		case 1: {
			HC595_SendByte(NumberTable[digBuffer[1]]);
			HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
			break;
		}
		case 2: {
			if (posPoint == 2)
				HC595_SendByte(NumberTable[digBuffer[2]] | DP);
			else
				HC595_SendByte(NumberTable[digBuffer[2]]);

			HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
			break;
		}
		case 3: {
			if (posPoint == 3)
				HC595_SendByte(NumberTable[digBuffer[3]] | DP);
			else
				HC595_SendByte(NumberTable[digBuffer[3]]);
			
			HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_SET);
			// HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
			// HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
			break;
		}
		default:	break;
	}
}

void displayStop(void)
{
	HAL_GPIO_WritePin(DIG0_GPIO_Port, DIG0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG1_GPIO_Port, DIG1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG2_GPIO_Port, DIG2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DIG3_GPIO_Port, DIG3_Pin, GPIO_PIN_RESET);
	HAL_TIM_Base_Stop_IT(&htim3);
}

void displayResume(void)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	HAL_TIM_Base_Start_IT(&htim3);
}







