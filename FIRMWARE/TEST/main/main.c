/**
 * @file main.c
 * @author Vanperdung (dung.nv382001@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

xTaskHandle task_handle;
TimerHandle_t timer;
static const char *TAG = "TEST";

void task_test(void *param)
{
    ESP_LOGI(TAG, "Create task %d", (uint32_t)xTaskGetTickCount() / portTICK_RATE_MS);
    while(1)
    {
        ESP_LOGI(TAG, "%d", (uint32_t)xTaskGetTickCount() / portTICK_RATE_MS);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void timer_cb(TimerHandle_t timer)
{
    vTaskDelete(task_handle);
    xTaskCreate(&task_test, "task", 4096, NULL, 10, &task_handle);
}

void app_main(void)
{
    timer = xTimerCreate("Heartbeat Gateway", 5000 / portTICK_RATE_MS, pdTRUE, (void *)0, timer_cb);
    xTimerStart(timer, 0);
    xTaskCreate(&task_test, "task", 4096, NULL, 10, &task_handle);
    while(1)
    {
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}
