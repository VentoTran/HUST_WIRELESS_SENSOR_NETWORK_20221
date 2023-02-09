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

#define LATCH_Port			CS1_GPIO_Port
#define LATCH_Pin			CS1_Pin
#define CLOCK_Port			SCK_GPIO_Port
#define CLOCK_Pin			SCK_Pin
#define DATA_Port			SER_GPIO_Port
#define DATA_Pin			SER_Pin

void HC595_SPI_Init(void);
void HC595_SPI_SendByte(uint8_t byte);

void HC595_Init(void);
void HC595_SendByte(uint8_t byte);

#endif /* INC_74HC595_H_ */
