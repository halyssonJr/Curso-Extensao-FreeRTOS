#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#define TASK_STACK_SIZE 1024*2
#define LED_BUILTIN 23
#define BTN 5
#define DEBOUNCE_COUNTER_MAX 5

#define PRESS_BTN BIT0
#define LED_ON BIT1

void init_hw();
void task_button( void * );
void task_led( void * );

static TaskHandle_t handle_task_button = NULL, handle_task_led = NULL;

EventGroupHandle_t event_group_handle;

void init_hw(){
    gpio_reset_pin(LED_BUILTIN);
    gpio_reset_pin(BTN);

    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN);

}

void task_button( void *pvParameters )
{
	uint8_t debounce_counter = 0;
    while(1)
	 {
        if (!gpio_get_level(BTN))
		  {
            debounce_counter++;
            if (debounce_counter > DEBOUNCE_COUNTER_MAX)
				{
					xTaskNotifyGive(handle_task_led);
					debounce_counter = 0;
				
					xEventGroupSetBits(event_group_handle, PRESS_BTN);
					while(!gpio_get_level(BTN));
				}
            vTaskDelay(pdMS_TO_TICKS(100));
        }
		  else
		  {
            debounce_counter = 0;
        }
        vTaskDelay(10);
    }
}

void task_led( void *pvParameters )
{
    uint32_t notified_value;
    while(1)
	 {
        if (xTaskNotifyWait(0,0, &notified_value, portMAX_DELAY)){
            printf("[task_led] fui notificado. %u\n", notified_value);
				if (notified_value > 3)
				{
					gpio_set_level(LED_BUILTIN, 1);
					xEventGroupSetBits(event_group_handle, LED_ON);
				}
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void task_event_group(void*pvParameters)
{
	EventBits_t event;
	while(1)
	{
		// 0000 0001 BIT0
		// 0000 0010 BIT1
		// ---------
		// 0000 0011
		event =  xEventGroupWaitBits(event_group_handle, (PRESS_BTN|LED_ON), pdFALSE, pdFALSE, portMAX_DELAY);
		
		if(event == PRESS_BTN)
		{
			printf ("Btn pressionado\n");
			//xEventGroupClearBits(event_group_handle, PRESS_BTN);
		}

		else if(event == LED_ON)
		{
			printf ("LED ON\n");
			//xEventGroupClearBits(event_group_handle, LED_ON);
		}

		vTaskDelay(pdMS_TO_TICKS(100));

	}
}
void app_main(void)
{
    
  init_hw();
  event_group_handle = xEventGroupCreate();

  xTaskCreate(task_button, "TASK_BUTTON", TASK_STACK_SIZE, NULL, 0, &handle_task_button);
  xTaskCreate(task_led, "TASK_LED", TASK_STACK_SIZE, NULL, 0, &handle_task_led);
	xTaskCreate(task_event_group, "event group", TASK_STACK_SIZE, NULL, 3, NULL );
}