/**
 * @file sx1278.c
 * @author Vanperdung (dung.nv382001@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-01-29
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "mqtt_client.h"
#include "esp_spiffs.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/spi_master.h"

#include "sx1278.h"
#include "uart_user.h"

static const char *TAG = "SX1278_GATEWAY";
static spi_device_handle_t spi_handle;
EventGroupHandle_t sx1278_evt_group;
sx1278_network_t sx1278_network = {0};
sx1278_attr_cfg_t attr_cfg_temp = {0};

void IRAM_ATTR sx1278_intr_handler(void *arg)
{
    gpio_num_t pin = (gpio_num_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (pin == SX1278_DIO0_PIN)
        xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO0_BIT, &xHigherPriorityTaskWoken);
    else if (pin == SX1278_DIO3_PIN)
        xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO3_BIT, &xHigherPriorityTaskWoken);
    else if (pin == SX1278_DIO4_PIN)
        xEventGroupSetBitsFromISR(sx1278_evt_group, SX1278_DIO4_BIT, &xHigherPriorityTaskWoken);
}

void sx1278_gpio_init(void)
{
    gpio_config_t io0_config = {
        .pin_bit_mask = BIT64(SX1278_DIO0_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&io0_config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(SX1278_DIO0_PIN, sx1278_intr_handler, (void *)SX1278_DIO0_PIN);

    gpio_config_t io3_config = {
        .pin_bit_mask = BIT64(SX1278_DIO3_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&io3_config);
    gpio_isr_handler_add(SX1278_DIO3_PIN, sx1278_intr_handler, (void *)SX1278_DIO3_PIN);

    gpio_config_t io4_config = {
        .pin_bit_mask = BIT64(SX1278_DIO4_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&io4_config);
    gpio_isr_handler_add(SX1278_DIO4_PIN, sx1278_intr_handler, (void *)SX1278_DIO4_PIN);
}

void sx1278_spi_init(void)
{
    spi_bus_config_t bus_config = {
        .mosi_io_num = SX1278_MOSI_PIN,
        .miso_io_num = SX1278_MISO_PIN,
        .sclk_io_num = SX1278_SCK_PIN,
        .max_transfer_sz = 0,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    assert(spi_bus_initialize(VSPI_HOST, &bus_config, 0) == ESP_OK);

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 4000000,
        .mode = 0,
        .spics_io_num = SX1278_NSS_PIN,
        .queue_size = 1,
        .pre_cb = NULL,
        .flags = 0,
    };
    assert(spi_bus_add_device(VSPI_HOST, &device_config, &spi_handle) == ESP_OK);
}

uint8_t sx1278_read_reg(uint8_t reg)
{
    uint8_t data_send[2] = {0x00 | reg, 0xFF}; // A wnr bit, which is 1 for write and 0 for read access
    uint8_t data_recv[2] = {0};
    spi_transaction_t sx1278_recv = {
        .flags = 0,
        .length = 8 * sizeof(data_send),
        .rx_buffer = (void *)data_recv,
        .tx_buffer = (void *)data_send,
    };
    gpio_set_level(SX1278_NSS_PIN, 0);
    spi_device_transmit(spi_handle, &sx1278_recv);
    gpio_set_level(SX1278_NSS_PIN, 1);
    return data_recv[1];
}

void sx1278_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t data_send[2] = {0x80 | reg, val}; // A wnr bit, which is 1 for write and 0 for read access
    uint8_t data_recv[2] = {0};
    spi_transaction_t sx1278_send = {
        .flags = 0,
        .length = 8 * sizeof(data_send),
        .rx_buffer = (void *)data_recv,
        .tx_buffer = (void *)data_send,
    };
    gpio_set_level(SX1278_NSS_PIN, 0);
    spi_device_transmit(spi_handle, &sx1278_send);
    gpio_set_level(SX1278_NSS_PIN, 1);
}

void sx1278_reset(void)
{
    gpio_set_level(SX1278_RST_PIN, 0);
    vTaskDelay(10 / portTICK_RATE_MS);
    gpio_set_level(SX1278_RST_PIN, 1);
    vTaskDelay(10 / portTICK_RATE_MS);
}

void sx1278_sleep(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void sx1278_standby(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void sx1278_rx_contiuous(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

void sx1278_rx_single(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE);
}

void sx1278_tx(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
}

void sx1278_cad(void)
{
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_CAD);
}

void sx1278_set_tx_power(uint8_t output_power)
{
    if (output_power > 15)
    {
        ESP_LOGE(TAG, "Invalid output power");
        return;
    }
    // PA output pin: PA_BOOST pin
    sx1278_write_reg(REG_PA_CONFIG, PA_BOOST | output_power);
}

void sx1278_set_LNA_gain(uint8_t gain)
{
    if (gain > 6)
    {
        ESP_LOGE(TAG, "Invalid gain");
        return;
    }

    if (gain == 0)
        sx1278_write_reg(REG_MODEM_CONFIG_3, 0x04);
    else
    {
        sx1278_write_reg(REG_MODEM_CONFIG_3, 0x00);
        sx1278_write_reg(REG_LNA, sx1278_read_reg(REG_LNA) | (gain << 5));
    }
}

void sx1278_set_freq(uint64_t freq)
{
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    sx1278_write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    sx1278_write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
    sx1278_write_reg(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

void sx1278_set_bandwidth(long band)
{
    int bw;
    if (band <= 7.8E3)
        bw = 0;
    else if (band <= 10.4E3)
        bw = 1;
    else if (band <= 15.6E3)
        bw = 2;
    else if (band <= 20.8E3)
        bw = 3;
    else if (band <= 31.25E3)
        bw = 4;
    else if (band <= 41.7E3)
        bw = 5;
    else if (band <= 62.5E3)
        bw = 6;
    else if (band <= 125E3)
        bw = 7;
    else if (band <= 250E3)
        bw = 8;
    else
        bw = 9;
    sx1278_write_reg(REG_MODEM_CONFIG_1, (sx1278_read_reg(REG_MODEM_CONFIG_1) & 0x0f) | (bw << 4));
}

void sx1278_set_sf(uint8_t sf)
{
    if (sf < 6 || sf > 12)
    {
        ESP_LOGE(TAG, "Invalid spreading factor");
        return;
    }

    if (sf == 6)
    {
        sx1278_write_reg(REG_DETECTION_OPTIMIZE, 0xc5);
        sx1278_write_reg(REG_DETECTION_THRESHOLD, 0x0c);
    }
    else
    {
        sx1278_write_reg(REG_DETECTION_OPTIMIZE, 0xc3);
        sx1278_write_reg(REG_DETECTION_THRESHOLD, 0x0a);
    }
    sx1278_write_reg(REG_MODEM_CONFIG_2, (sx1278_read_reg(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));
}

void sx1278_set_cr(uint8_t cr)
{
    if (cr < 5 || cr > 8)
    {
        ESP_LOGE(TAG, "Invalid coding rate");
        return;
    }

    cr = cr - 4;
    sx1278_write_reg(REG_MODEM_CONFIG_1, (sx1278_read_reg(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
}

void sx1278_set_header(bool en, uint32_t size)
{
    if (en)
        sx1278_write_reg(REG_MODEM_CONFIG_1, sx1278_read_reg(REG_MODEM_CONFIG_1) & 0xfe);
    else
    {
        sx1278_write_reg(REG_MODEM_CONFIG_1, sx1278_read_reg(REG_MODEM_CONFIG_1) | 0x01);
        sx1278_write_reg(REG_PAYLOAD_LENGTH, size);
    }
}

void sx1278_set_crc(bool en)
{
    if (en)
        sx1278_write_reg(REG_MODEM_CONFIG_2, sx1278_read_reg(REG_MODEM_CONFIG_2) | 0x04);
    else
        sx1278_write_reg(REG_MODEM_CONFIG_2, sx1278_read_reg(REG_MODEM_CONFIG_2) & 0xfb);
}

void sx1278_set_preamble(int len)
{
    sx1278_write_reg(REG_PREAMBLE_MSB, (uint8_t)(len >> 8));
    sx1278_write_reg(REG_PREAMBLE_LSB, (uint8_t)(len >> 0));
}

int sx1278_get_rssi(void)
{
    return (sx1278_read_reg(REG_PKT_RSSI_VALUE) - 164);
}

float sx1278_get_snr(void)
{
    return ((int8_t)sx1278_read_reg(REG_PKT_SNR_VALUE) * 0.25);
}

void sx1278_set_irq(uint8_t val)
{
    sx1278_write_reg(REG_DIO_MAPPING_1, val);
}

void sx1278_init(void)
{
    sx1278_reset();
    uint8_t ver = sx1278_read_reg(REG_VERSION);
    ESP_LOGI(TAG, "SX1278 version: 0x%02x", ver);
    sx1278_standby();
    sx1278_write_reg(REG_FIFO_RX_BASE_ADDR, 0x00);
    sx1278_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    sx1278_set_LNA_gain(0);
    sx1278_set_tx_power(8); // Pout = 10 dBm (10 mW)
    sx1278_set_freq(433E6);
    sx1278_set_bandwidth(250E3); // Bandwidth: 250 kHz
    sx1278_set_sf(9);
    sx1278_set_cr(5);
    sx1278_set_preamble(12);
    sx1278_set_header(true, 0);
    sx1278_set_crc(true);
    sx1278_set_irq(0x00);
    sx1278_sleep();
}

void sx1278_send_data(uint8_t *data_send, int size)
{
    sx1278_standby();
    sx1278_write_reg(REG_FIFO_ADDR_PTR, 0);
    for (int index = 0; index < size; index++)
    {
        sx1278_write_reg(REG_FIFO, data_send[index]);
    }
    sx1278_write_reg(REG_PAYLOAD_LENGTH, size);
    // Start transmission and wait for conclusion
    sx1278_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
    while (!(sx1278_read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK))
    {
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    sx1278_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
    sx1278_sleep();
}

void sx1278_start_recv_data(void)
{
    sx1278_rx_contiuous();
}

sx1278_err_t sx1278_recv_data(uint8_t *data_recv, int *rssi, float *snr, sx1278_packet_t *packet)
{
    int irq = sx1278_read_reg(REG_IRQ_FLAGS);
    memset((char *)data_recv, '\0', strlen((char *)data_recv));
    sx1278_write_reg(REG_IRQ_FLAGS, irq);

    if (!(irq & IRQ_RX_DONE_MASK))
    {
        ESP_LOGE(TAG, "Invalid RxDone Interrupt");
        return SX1278_INVALID_RX_DONE;
    }

    if (!(irq & IRQ_VALID_HEADER_MASK))
    {
        ESP_LOGE(TAG, "Invalid Header Interrupt");
        return SX1278_INVALID_HEADER;
    }

    if (irq & IRQ_PAYLOAD_CRC_ERROR_MASK)
    {
        ESP_LOGE(TAG, "Payload Crc Error Interrupt");
        return SX1278_PAYLOAD_CRC_ERROR;
    }

    int len = sx1278_read_reg(REG_RX_NB_BYTES);
    *rssi = sx1278_get_rssi();
    *snr = sx1278_get_snr();
    sx1278_standby();
    sx1278_write_reg(REG_FIFO_ADDR_PTR, sx1278_read_reg(REG_FIFO_RX_CURRENT_ADDR));
    for (int index = 0; index < len; index++)
    {
        data_recv[index] = sx1278_read_reg(REG_FIFO);
    }
    sx1278_sleep();
    parse_packet(data_recv, packet);
    return SX1278_OK;
}

int get_random_value(int min, int max)
{
    int random;
    if (max <= min)
    {
        ESP_LOGE(TAG, "Range error");
        return 0;
    }
    return random = min + rand() % (max + 1 - min);
}

uint8_t get_crc_value(uint8_t *data, int len)
{
    uint8_t crc = 0;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
    }
    return crc;
}

void send_request(sx1278_opcode_type_t opcode, sx1278_node_slot_t node_slot, uint8_t *packet)
{
    memset((char *)packet, '\0', strlen((char *)packet));
    packet[1] = (uint8_t)(opcode & 0x00FF);
    packet[0] = (uint8_t)((opcode & 0xFF00) >> 8);
    packet[3] = (uint8_t)(node_slot.node_id & 0x00FF);
    packet[2] = (uint8_t)((node_slot.node_id & 0xFF00) >> 8);
    packet[5] = (uint8_t)(sx1278_network.gate_id & 0x00FF);
    packet[4] = (uint8_t)((sx1278_network.gate_id & 0xFF00) >> 8);
    if (opcode == UPLINK_TX_REQUEST_OPCODE)
    {
        for (int i = 0; i < sizeof(float); i++)
        {
            packet[6 + i] = node_slot.period.bytes[i];
        }
        for (int i = 0; i < sizeof(float); i++)
        {
            packet[10 + i] = node_slot.threshold.bytes[i];
        }
        packet[14] = get_crc_value(packet, 14);
    }
    sx1278_send_data(packet, strlen((char *)packet));
}

sx1278_err_t parse_packet(uint8_t *packet_data, sx1278_packet_t *packet)
{
    int packet_len = strlen((char *)packet_data);
    packet->opcode = (uint16_t)(packet_data[0] << 8 | packet_data[1]);
    packet->node_id = (uint16_t)(packet_data[2] << 8 | packet_data[3]);
    packet->gate_id = (uint16_t)(packet_data[4] << 8 | packet_data[5]);
    if (packet->opcode == DOWNLINK_RX_DATA_OPCODE)
    {
        if (packet_len != 23)
        {
            ESP_LOGE(TAG, "Error packet len, opcode: 0x%04x", DOWNLINK_RX_DATA_OPCODE);
            return SX1278_NOT_OK;
        }
        else
        {
            memcpy((char *)packet->temp.bytes, (char *)&packet_data[6], sizeof(float));
            memcpy((char *)packet->battery.bytes, (char *)&packet_data[10], sizeof(float));
            memcpy((char *)packet->period.bytes, (char *)&packet_data[14], sizeof(float));
            memcpy((char *)packet->threshold.bytes, (char *)&packet_data[18], sizeof(float));
            packet->crc = packet_data[23];
            int crc_check = get_crc_value(packet_data, strlen((char *)packet_data));
            if (crc_check != packet->crc)
            {
                ESP_LOGE(TAG, "Error packet crc, opcode 0x%04x", DOWNLINK_RX_DATA_OPCODE);
                return SX1278_NOT_OK;
            }
            else
            {
                ESP_LOGI(TAG, "Recv opcode 0x%04x from node_id: 0x%04x", packet->opcode, packet->node_id);
                ESP_LOGI(TAG, "temp: %f, battery: %f, threshold: %f, period: %f", packet->temp.float_val, packet->battery.float_val, packet->threshold.float_val, packet->period.float_val);
                return SX1278_OK;
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "Error opcode: 0x%04x", packet->opcode);
        return SX1278_NOT_OK;
    }
}

bool listen_before_talk(void)
{
    EventBits_t evt_bits;
    while (1)
    {
        sx1278_cad();
        evt_bits = xEventGroupWaitBits(sx1278_evt_group, SX1278_DIO3_BIT | SX1278_DIO4_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if (evt_bits & SX1278_DIO3_BIT) // CadDone
        {
            ESP_LOGW(TAG, "Cad timeout");
            return true;
        }
        else if (evt_bits & SX1278_DIO4_BIT) // CadDetected
        {
            sx1278_sleep();
            TickType_t time_delay = (TickType_t)get_random_value(0, 50);
            ESP_LOGW(TAG, "Cad Detected, sleep in %d", (int)time_delay);
            vTaskDelay(time_delay / portTICK_RATE_MS);
        }
    }
}

void sx1278_task(void *param)
{
    uint8_t data_send[128] = {0};
    uint8_t data_recv[128] = {0};
    float snr;
    int rssi;
    EventBits_t evt_bits;
    sx1278_packet_t packet;
    TickType_t period_tick;
    int slot_cnt = 0;
    sx1278_gpio_init();
    sx1278_spi_init();
    sx1278_init();
    sx1278_evt_group = xEventGroupCreate();
    sx1278_network.total_slots = NW_DEFAULT_TOTAL_SLOTS;
    while (1)
    {
        if (sx1278_network.flags.network_run == true)
        {
            if (slot_cnt == 0)
            {
                if (attr_cfg_temp.period.float_val < 5)
                {
                    for (int i = 0; i < NW_DEFAULT_TOTAL_SLOTS; i++)
                    {
                        sx1278_network.node_slots[i].period.float_val = NW_DEFAULT_PERIOD;
                    }
                }
                else
                {
                    for (int i = 0; i < NW_DEFAULT_TOTAL_SLOTS; i++)
                    {
                        sx1278_network.node_slots[i].period.float_val = attr_cfg_temp.period.float_val;
                    }
                }
                for (int i = 0; i < NW_DEFAULT_TOTAL_SLOTS; i++)
                {
                    sx1278_network.node_slots[i].threshold.float_val = attr_cfg_temp.threshold[i].float_val;
                }
            }
            period_tick = xTaskGetTickCount();
            ESP_LOGI(TAG, "Period: %d", (int)sx1278_network.node_slots[slot_cnt].period.float_val);
            while ((uint32_t)(xTaskGetTickCount() - period_tick) / 1000 < ((uint32_t)sx1278_network.node_slots[slot_cnt].period.float_val) / portTICK_RATE_MS)
            {
                listen_before_talk();
                ESP_LOGI(TAG, "Send opcode 0x%04x", UPLINK_TX_REQUEST_OPCODE);
                send_request(UPLINK_TX_REQUEST_OPCODE, sx1278_network.node_slots[slot_cnt], data_send);
                ESP_LOGI(TAG, "Start recv opcode");
                sx1278_start_recv_data();
                evt_bits = xEventGroupWaitBits(sx1278_evt_group, SX1278_DIO0_BIT, pdTRUE, pdFALSE, 1000 / portTICK_PERIOD_MS);
                if (evt_bits & SX1278_DIO0_BIT) // RxDone
                {
                    if (sx1278_recv_data(data_recv, &rssi, &snr, &packet) == SX1278_OK)
                    {
                        // DO SOMETHING
                        while ((uint32_t)(xTaskGetTickCount() - period_tick) / 1000 < ((uint32_t)sx1278_network.node_slots[slot_cnt].period.float_val) / portTICK_RATE_MS)
                        {
                            vTaskDelay(10 / portTICK_RATE_MS);
                        }
                    }
                    else
                        continue;
                }
                else
                    continue;
            }
            slot_cnt++;
            if (slot_cnt == 10)
                slot_cnt = 0;
        }
        else
        {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
}
