#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_log.h"

#define PIN_IN_BTN GPIO_NUM_0
#define PIN_OUT_LED GPIO_NUM_23
#define PIN_IN_ADC GPIO_NUM_36
#define POT_ADC_CHANNEL ADC1_CHANNEL_0

#define TASK_DELAY_TIME_DEFAULT pdMS_TO_TICKS(10)

TaskHandle_t hnd_task_btn = NULL;
TaskHandle_t hnd_task_led = NULL;
TaskHandle_t hnd_task_adc = NULL;

void init_hw(void)
{
    //
    // Configure GPIOs
    //
    esp_err_t err = ESP_OK;

    // Button
    gpio_config_t io_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .pin_bit_mask = 1 << PIN_IN_BTN,
    };
    err = gpio_config(&io_config);
    if (err != ESP_OK)
    {
        ESP_LOGE("inithw", "gpio_config failed: %d", err);
    }

    // LED
    io_config = (gpio_config_t){
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .pin_bit_mask = 1 << PIN_OUT_LED,
    };
    err = gpio_config(&io_config);
    if (err != ESP_OK)
    {
        ESP_LOGE("inithw", "gpio_config failed: %d", err);
    }

    //
    // Configure ADC Channels
    //
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

void task_btn(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(TASK_DELAY_TIME_DEFAULT);
    }
}

void task_led(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(TASK_DELAY_TIME_DEFAULT);
    }
}

void task_adc(void *pvParameters)
{
    uint32_t adc_value = 0;
    while(1)
    {
        adc_value = adc1_get_raw(ADC1_CHANNEL_0);
        ESP_LOGI("adc", "adc_value: %d", adc_value);
        vTaskDelay(TASK_DELAY_TIME_DEFAULT * 100);
    }
}

void task_monitor(void *pvParameters)
{
    while(1)
    {
        uint32_t free_on_task_btn = uxTaskGetStackHighWaterMark(hnd_task_btn);
        uint32_t free_on_task_led = uxTaskGetStackHighWaterMark(hnd_task_led);
        uint32_t free_on_task_adc = uxTaskGetStackHighWaterMark(hnd_task_adc);
        uint32_t num_of_tasks = uxTaskGetNumberOfTasks();
        ESP_LOGI("monitor", "free_on_task_btn: %d", free_on_task_btn);
        ESP_LOGI("monitor", "free_on_task_led: %d", free_on_task_led);
        ESP_LOGI("monitor", "free_on_task_adc: %d", free_on_task_adc);
        ESP_LOGI("monitor", "num_of_tasks: %d", num_of_tasks);
        // Get task list
        TaskStatus_t *task_list = (TaskStatus_t *)malloc(sizeof(TaskStatus_t) * num_of_tasks);
        if (task_list == NULL)
        {
            ESP_LOGE("monitor", "malloc failed");
            vTaskDelay(TASK_DELAY_TIME_DEFAULT * 100);
            continue;
        }
        uint32_t num_of_tasks_returned = uxTaskGetSystemState(task_list, num_of_tasks, NULL);
        ESP_LOGI("monitor", "num_of_tasks_returned: %d", num_of_tasks_returned);
        for (int i = 0; i < num_of_tasks_returned; i++)
        {
            ESP_LOGI("monitor", "task_list[%d].xTaskNumber: %s", i, task_list[i].pcTaskName);
        }
        free(task_list);
        vTaskDelay(TASK_DELAY_TIME_DEFAULT * 100);
    }
}

void app_main(void)
{
    init_hw();
    xTaskCreate(task_btn, "task_btn", 2048, NULL, 1, &hnd_task_btn);
    xTaskCreate(task_led, "task_led", 2048, NULL, 1, &hnd_task_led);
    xTaskCreate(task_adc, "task_adc", 2048, NULL, 1, &hnd_task_adc);
    xTaskCreate(task_monitor, "task_monitor", 2048, NULL, 1, NULL);
}