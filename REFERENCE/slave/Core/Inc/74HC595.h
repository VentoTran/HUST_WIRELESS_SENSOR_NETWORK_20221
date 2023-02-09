/*
 * 74HC595.h
 *
 *  Created on: Feb 6, 2023
 *      Author: Admin
 */

#ifndef INC_74HC595_H_
#define INC_74HC595_H_

#include "stm32f1xx.h"
#include "gpio.h"

#define LATCH_Port			GPIOB
#define LATCH_Pin			12
#define CLOCK_Port			GPIOB
#define CLOCK_Pin			13
#define DATA_Port			GPIOB
#define DATA_Pin			14

void HC595_SPI_Init(void);
void HC595_SPI_SendByte(uint8_t byte);

void HC595_Init(void);
void HC595_SendByte(uint8_t byte);

#endif /* INC_74HC595_H_ */
