#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_BUILTIN 2
#define BTN         5

SemaphoreHandle_t semphr_bin_handler;
SemaphoreHandle_t semphr_count_handler;

volatile TickType_t btn_deounce;

int meu_precioso = 0;
uint8_t led_status = 0;

void init_hw(void)
{
  // Configurando botão como entrada digital
  gpio_set_direction(BTN, GPIO_MODE_INPUT);

  //Configurando LED como saída digital
  gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);

}

void vtask_flash_led (void *pvParameters)
{

  while(1)
  {
  // pdTRUE = 1
  // pdFALSE = 0
    if(xSemaphoreTake(semphr_bin_handler, pdMS_TO_TICKS(2000)) == pdTRUE)
    {
      if(xTaskGetTickCount() - btn_deounce < 500)
      {
        led_status = !led_status;
        gpio_set_level(LED_BUILTIN, led_status);
        printf("Mudou de estado\n");
      }      

    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

void vtask_button (void *pvParameters)
{
  while(1)
  {
    if(gpio_get_level(BTN) == 0)
    {
      btn_deounce = xTaskGetTickCount();
      xSemaphoreGive(semphr_bin_handler);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void vtask_1 (void *pvParameters)
{
  while(1)
  {
    if(xSemaphoreTake(semphr_count_handler, portMAX_DELAY) == pdTRUE)
    {
      meu_precioso ++;
      printf("Task 1 esta com meu precioso = %d\n", meu_precioso);
      xSemaphoreGive(semphr_count_handler);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void vtask_2 (void *pvParameters)
{
  while (1)
  {
    if(xSemaphoreTake(semphr_count_handler, portMAX_DELAY) == pdTRUE)
    {
      meu_precioso --;
      printf("Task 2 esta com meu precioso = %d\n", meu_precioso);
      xSemaphoreGive(semphr_count_handler);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  
}

void app_main(void)
{
  init_hw();

  semphr_bin_handler = xSemaphoreCreateBinary();
  if(semphr_bin_handler == NULL)
  {
    printf ("Erro, falha ao criar o semaforo\n");
    return;
  }

  xTaskCreate (vtask_flash_led, "flash LED", 4098, NULL, 2, NULL);
  xTaskCreate (vtask_button, "button", 2048, NULL, 2, NULL);

  semphr_count_handler = xSemaphoreCreateCounting(1 , 1);
  if(semphr_count_handler == NULL)
  {
    printf ("Erro, falha ao criar o semaforo\n");
    return;
  }

  xTaskCreate (vtask_1, "TASK 1", 1024*2, NULL, 2, NULL);
  xTaskCreate (vtask_2, "TASK 2", 1024*2, NULL, 2, NULL);

}