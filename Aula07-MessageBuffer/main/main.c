  #include <stdio.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "freertos/stream_buffer.h"
  #include "freertos/message_buffer.h"
  #include "driver/gpio.h"
  #include "driver/adc.h"
  #include "esp_log.h"

#define BTN 5
#define LED 2
#define MESSAGE_BUFFER_SIZE 50
#define TAG "Aula 07"

MessageBufferHandle_t xmsg_buffer;

// ADC1_CHANNEL_0 = 0, /*!< ADC1 channel 0 is GPIO36 */
// ADC1_CHANNEL_1,     /*!< ADC1 channel 1 is GPIO37 */
// ADC1_CHANNEL_2,     /*!< ADC1 channel 2 is GPIO38 */
// ADC1_CHANNEL_3,     /*!< ADC1 channel 3 is GPIO39 */
// ADC1_CHANNEL_4,     /*!< ADC1 channel 4 is GPIO32 */
// ADC1_CHANNEL_5,     /*!< ADC1 channel 5 is GPIO33 */
// ADC1_CHANNEL_6,     /*!< ADC1 channel 6 is GPIO34 */
// ADC1_CHANNEL_7,     /*!< ADC1 channel 7 is GPIO35 */


void init_hw(void)
{
  gpio_config_t g_config =
  {
    .intr_type = GPIO_INTR_DISABLE,
    .mode  = GPIO_MODE_INPUT,
    .pull_up_en = 1,
    .pull_down_en = 0,
    .pin_bit_mask = (1<<BTN),
  };
  gpio_config (&g_config);

  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

void init_analog(void)
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
}

void task_btn (void *pvParameters)
{
  bool btn_is_press = false;
  uint32_t counter_press = 0;

  while(1)
  {
      if(!gpio_get_level(BTN) && !btn_is_press)
      {
        vTaskDelay(pdMS_TO_TICKS(50));
        ESP_LOGI(TAG, "Pressionou o botão");
        counter_press ++;
        xMessageBufferSend(xmsg_buffer,(void*)&counter_press, sizeof(counter_press), 0);
        btn_is_press = true;
      }
      else if (gpio_get_level(BTN) && btn_is_press)
      {
        ESP_LOGI(TAG, "Botão solto");
        btn_is_press = false;
      }
      vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void task_led (void *pvParameters)
{
  uint32_t counter_btn = 0;
  while(1)
  {
    if (xMessageBufferReceive(xmsg_buffer, (void *)&counter_btn, sizeof(xmsg_buffer), portMAX_DELAY) > 0)
    {
      ESP_LOGW(TAG, "Counter = %d", counter_btn);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void task_adc(void *pvParameters)
{
  init_analog();
  int analog_value;
  while(1)
  {
    analog_value = adc1_get_raw(ADC1_CHANNEL_0);
    ESP_LOGI(TAG, "Valor = %d ",analog_value);// 0 - 4095 (12 bits 2^12)

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void)
{
  init_hw();
  xmsg_buffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);

  xTaskCreate(task_btn, "task btn", 1024*3, NULL, 2, NULL);
  xTaskCreate(task_led, "task led", 1024*3, NULL, 2, NULL);
  xTaskCreate(task_adc, "task_adc", 1024*3, NULL, 2, NULL);
}