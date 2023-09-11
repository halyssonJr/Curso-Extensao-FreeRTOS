#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BTN 5
#define BTN_UP 18
#define LED 2

#define BTNS ((1 << BTN) | (1 << BTN_UP))
// ... 0000 0110 0000
#define TAG "AULA 06"
QueueHandle_t xqueue_btn;

void init_hw(void)
{
  gpio_config_t g_config =
  {
    .intr_type = GPIO_INTR_DISABLE,
    .mode  = GPIO_MODE_INPUT,
    .pull_up_en = 1,
    .pull_down_en = 0,
    .pin_bit_mask = BTNS,
  };
  gpio_config (&g_config);

  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
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
         if( xQueueSend(xqueue_btn, &counter_press, 0)== pdTRUE)
         {
            ESP_LOGW(TAG, "Enviei o counter press");
         }
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
      if(xQueueReceive(xqueue_btn, &counter_btn, portMAX_DELAY) == pdTRUE)
      {
         ESP_LOGE(TAG, "Counter = %d", counter_btn);
      }

      vTaskDelay(pdMS_TO_TICKS(100));
   }
}
void app_main(void)
{
   init_hw();
   
   xqueue_btn = xQueueCreate(5, sizeof(uint32_t));
   if (xqueue_btn == NULL)
   {
      ESP_LOGE(TAG, "Erro ao alocar a fila");
      return;
   }

   xTaskCreate(task_btn, "task btn", 1024*3, NULL, 2, NULL);
   xTaskCreate(task_led, "task led", 1024*3, NULL, 2, NULL);
}