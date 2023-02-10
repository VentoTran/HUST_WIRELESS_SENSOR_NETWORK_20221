/**
 * @file display.h
 * @author Vento (zseefvhu12345@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2023-02-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "main.h"
#include "74HC595.h"
#include "string.h"
#include "tim.h"

extern uint8_t digIndex;

void displayInit(void);
void displayDigit(uint8_t dig);
void displayFloat(float f);
void displayStop(void);
void displayResume(void);


#endif  //_DISPLAY_H_