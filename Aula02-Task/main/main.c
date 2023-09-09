#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#define LED_BUILTIN 4
#define LED_GREEN  5

TaskHandle_t xhandle_task1;
TaskHandle_t xhandle_task2;

void task_1 (void *pvParameters)
{
   printf ("Iniciou task 1\n");
   
   for(;;)
   {
      gpio_set_level(LED_BUILTIN,1);     
      vTaskDelay(pdMS_TO_TICKS(1000));
      
      gpio_set_level(LED_BUILTIN,0);
      vTaskDelay(pdMS_TO_TICKS(1000));
   }

}

void task_2 (void *pvParameters)
{
   printf ("Iniciou task 2\n");

   for(;;)
   {

      gpio_set_level(LED_GREEN,1);     
      vTaskDelay(pdMS_TO_TICKS(500));
      
      gpio_set_level(LED_GREEN,0);
      vTaskDelay(pdMS_TO_TICKS(500)); 
   }
   printf("Task 2 excluida\n");
   vTaskDelete(NULL);
}

void init_hw(void)
{
   //Inicialização do LED
   gpio_pad_select_gpio(LED_BUILTIN);
   gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
   gpio_pad_select_gpio(LED_GREEN);
   gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
   init_hw(); // Inicializando as gpios
   int ret;

   ret = xTaskCreate(task_1,       // Task
               "task 1",           // Nome da task        
               1024,               // Quantidade em bytes de memoria consumida    
               NULL,               // Parametros
               2,                  // Prioridade
               &xhandle_task1);    // Handler
   if (ret == pdFALSE)
   {
     printf("Erro ao criar a task 1\n");
     return;
   }
 
   ret = xTaskCreate(  task_2,      // Task
               "task 2",            // Nome da task        
               1024,                // Quantidade em bytes de memoria consumida    
               NULL,                // Parametros
               2,                   // Prioridade
               &xhandle_task2);     // Handler 

   if (ret == pdFALSE)
   {
     printf("Erro ao criar a task 1\n");
     return;
   }
   
   printf ("Tasks criadas com sucesso \n");
}
