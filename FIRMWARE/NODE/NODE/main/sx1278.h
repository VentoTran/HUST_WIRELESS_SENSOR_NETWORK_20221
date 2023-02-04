/**
 * @file sx1278.h
 * @author Vanperdung (dung.nv382001@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#ifndef _SX1278_H_
#define _SX1278_H_

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SX1278_MOSI_PIN GPIO_NUM_23
#define SX1278_MISO_PIN GPIO_NUM_19
#define SX1278_NSS_PIN GPIO_NUM_14
#define SX1278_SCK_PIN GPIO_NUM_18
#define SX1278_EXTI_PIN GPIO_NUM_32
#define SX1278_RST_PIN GPIO_NUM_33

// Register definitions
#define REG_FIFO                        0x00
#define REG_OP_MODE                     0x01
#define REG_FRF_MSB                     0x06
#define REG_FRF_MID                     0x07
#define REG_FRF_LSB                     0x08
#define REG_PA_CONFIG                   0x09
#define REG_LNA                         0x0c
#define REG_FIFO_ADDR_PTR               0x0d
#define REG_FIFO_TX_BASE_ADDR           0x0e
#define REG_FIFO_RX_BASE_ADDR           0x0f
#define REG_FIFO_RX_CURRENT_ADDR        0x10
#define REG_IRQ_FLAGS                   0x12
#define REG_RX_NB_BYTES                 0x13
#define REG_PKT_SNR_VALUE               0x19
#define REG_PKT_RSSI_VALUE              0x1a
#define REG_MODEM_CONFIG_1              0x1d
#define REG_MODEM_CONFIG_2              0x1e
#define REG_PREAMBLE_MSB                0x20
#define REG_PREAMBLE_LSB                0x21
#define REG_PAYLOAD_LENGTH              0x22
#define REG_MODEM_CONFIG_3              0x26
#define REG_RSSI_WIDEBAND               0x2c
#define REG_DETECTION_OPTIMIZE          0x31
#define REG_DETECTION_THRESHOLD         0x37
#define REG_SYNC_WORD                   0x39
#define REG_DIO_MAPPING_1               0x40
#define REG_VERSION                     0x42

// Transceiver modes
#define MODE_LONG_RANGE_MODE            0x80
#define MODE_SLEEP                      0x00
#define MODE_STDBY                      0x01
#define MODE_TX                         0x03
#define MODE_RX_CONTINUOUS              0x05
#define MODE_RX_SINGLE                  0x06

// PA configuration
#define PA_BOOST                        0x80

// IRQ masks
#define IRQ_TX_DONE_MASK                0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK      0x20
#define IRQ_RX_DONE_MASK                0x40

#define PA_OUTPUT_RFO_PIN               0
#define PA_OUTPUT_PA_BOOST_PIN          1

#define TIMEOUT_RESET                   100

#define BUFF "VANPERDUNG VANPERDUNG"

void sx1278_task(void *param);

#endif