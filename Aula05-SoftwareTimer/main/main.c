#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#define TIMEOUT BIT0
#define TIMER_STOP BIT1


TimerHandle_t xtimer_led; 
EventGroupHandle_t xevent_group_timer;


void vTimer_LED_cb(TimerHandle_t xtimer)
{
   uint32_t counter  = (uint32_t)pvTimerGetTimerID(xtimer);
   counter--;
   printf("Timer estorou, counter = %u \n", counter);
   
   if(counter <= 0)
   {
     xTimerStop(xtimer_led, 0);
     xEventGroupSetBits(xevent_group_timer, TIMER_STOP);
   }
   else 
   {
      vTimerSetTimerID(xtimer, (void *)counter);
   }
   xEventGroupSetBits(xevent_group_timer, TIMEOUT);

}

void app_main(void)
{
  
   xtimer_led = xTimerCreate("Timer do LED", pdMS_TO_TICKS(500), pdTRUE, (void *)10, vTimer_LED_cb);
   xevent_group_timer = xEventGroupCreate();

   xTimerStart(xtimer_led, 0);
   EventBits_t event;

   while(1)
   {
      event = xEventGroupWaitBits(xevent_group_timer, (TIMEOUT | TIMER_STOP), pdTRUE, pdFALSE, portMAX_DELAY);
      
      if (event == TIMEOUT)
      {
         printf ("Timeout !!! \n");
      }
      else if (event == TIMER_STOP)
      {
         printf ("Timer excluido :) \n");
         xTimerDelete(xtimer_led, 0);
      }
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}