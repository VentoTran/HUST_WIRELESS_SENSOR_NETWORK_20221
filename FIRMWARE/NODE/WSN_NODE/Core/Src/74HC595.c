/*
 * 74HC595.c
 *
 *  Created on: Feb 6, 2023
 *      Author: Admin
 */

#include "74HC595.h"

void HC595_SPI_Init(void) {

	GPIO_Enable(LATCH_Port);							// Enable port
	GPIO_SetMode_Output_2MHz_PP(LATCH_Port, LATCH_Pin);	// Init latch pin
	GPIO_WritePin(LATCH_Port, LATCH_Pin, 0);			// Low level
}

void HC595_SPI_SendByte(uint8_t byte) {

	uint16_t counter = 1000;

	SPI1_Write(byte);

	// Toggle latch pin
	GPIO_WritePin(LATCH_Port, LATCH_Pin, 1);
	while (counter--);
	GPIO_WritePin(LATCH_Port, LATCH_Pin, 0);
}

void HC595_Init(void)
{
	HAL_GPIO_WritePin(CLOCK_Port, CLOCK_Pin, 0);
	HAL_GPIO_WritePin(LATCH_Port, LATCH_Pin, 0);
	HAL_GPIO_WritePin(DATA_Port, DATA_Pin, 0);
}

void HC595_SendByte(uint8_t byte)
{
	uint16_t clock_timeout = 10000;
	uint16_t latch_timeout = 1000;

	for (int8_t i = 7; i >= 0; --i) {

		uint8_t bit = byte & (0x1 << i);				// Read bit
		HAL_GPIO_WritePin(DATA_Port, DATA_Pin, bit);		// Send bit

		// Toggle clock
		HAL_GPIO_WritePin(CLOCK_Port, CLOCK_Pin, 1);
		while(clock_timeout--);
		HAL_GPIO_WritePin(CLOCK_Port, CLOCK_Pin, 0);
	}

	// Toggle latch
	HAL_GPIO_WritePin(LATCH_Port, LATCH_Pin, 1);
	while (latch_timeout--);
	HAL_GPIO_WritePin(LATCH_Port, LATCH_Pin, 0);
}


