/*
 * onewire.h
 *
 *  Created on: Jan 20, 2023
 *      Author: PhongEE
 */

#ifndef INC_ONEWIRE_H_
#define INC_ONEWIRE_H_

#include "gpio.h"
#include "tim.h"

#define DS18B20_GPIO_Port 	(GPIOB)
#define DS18B20_Pin  		(GPIO_PIN_0)

typedef struct {
	GPIO_TypeDef* GPIOx;           // Bus GPIO Port
	uint16_t GPIO_Pin;             // Bus GPIO Pin
	uint8_t LastDiscrepancy;       // For searching purpose
	uint8_t LastFamilyDiscrepancy; // For searching purpose
	uint8_t LastDeviceFlag;        // For searching purpose
	uint8_t ROM_NO[8];             // 8-byte ROM addres last found device
} OneWire_t;

#define ONEWIRE_CMD_RSCRATCHPAD			0xBE
#define ONEWIRE_CMD_WSCRATCHPAD			0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD		0x48
#define ONEWIRE_CMD_RECEEPROM			0xB8
#define ONEWIRE_CMD_RPWRSUPPLY			0xB4
#define ONEWIRE_CMD_SEARCHROM			0xF0
#define ONEWIRE_CMD_READROM				0x33
#define ONEWIRE_CMD_MATCHROM			0x55
#define ONEWIRE_CMD_SKIPROM				0xCC

void OneWire_Init(OneWire_t* OneWireStruct, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t OneWire_Reset(OneWire_t* OneWireStruct);
void OneWire_ResetSearch(OneWire_t* OneWireStruct);
uint8_t OneWire_First(OneWire_t* OneWireStruct);
uint8_t OneWire_Next(OneWire_t* OneWireStruct);
uint8_t OneWire_Search(OneWire_t* OneWireStruct, uint8_t command);
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit);
uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct);
void OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte);
uint8_t OneWire_ReadByte(OneWire_t* OneWireStruct);
void OneWire_GetFullROM(OneWire_t* OneWireStruct, uint8_t *firstIndex);
void OneWire_Select(OneWire_t* OneWireStruct, uint8_t* addr);
void OneWire_SelectWithPointer(OneWire_t* OneWireStruct, uint8_t* ROM);
uint8_t OneWire_CRC8(uint8_t* addr, uint8_t len);


#endif /* INC_ONEWIRE_H_ */
