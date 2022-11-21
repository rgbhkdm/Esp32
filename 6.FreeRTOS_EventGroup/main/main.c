#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

EventGroupHandle_t event_group;
const int task0_bit = BIT0;
const int task1_bit = BIT1;
const int task2_bit = BIT2;
const int ALL_SYNC_BITS=task0_bit|task1_bit|task2_bit;

void task0(void * pvParameters)
{
    EventBits_t uxReturn;
    TickType_t xTicksToWait = 5000 / portTICK_PERIOD_MS;

    while (1)
    {
        // xEventGroupSetBits(event_group, got_temp);
        uxReturn=xEventGroupSync(event_group, task0_bit, ALL_SYNC_BITS, xTicksToWait);
       
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("%d\n",uxReturn);

        if ((uxReturn & ALL_SYNC_BITS) == ALL_SYNC_BITS)
        {
            // All three tasks reached the synchronisation point before the call
            // to xEventGroupSync() timed out.
            printf("task0\n");
        }
    }
}

void task1(void * pvParameters)
{
    EventBits_t uxReturn;
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //xEventGroupSetBits(event_group, got_temp);
        uxReturn=xEventGroupSync(event_group, task1_bit, ALL_SYNC_BITS, portMAX_DELAY);
        //printf("%d\n",uxReturn);
        printf("task1\n");
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task2(void * pvParameters)
{
    while (1)
    {        
        //xEventGroupSetBits(event_group, got_temp);
        xEventGroupSync(event_group, task2_bit, ALL_SYNC_BITS, portMAX_DELAY);
        printf("task2\n");
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    event_group = xEventGroupCreate();
    xTaskCreate(task0, "task0", 2048, NULL, 1, NULL);
    vTaskDelay(10/ portTICK_PERIOD_MS);
    xTaskCreate(task1, "task1", 2048, NULL, 1, NULL);
    vTaskDelay(10/ portTICK_PERIOD_MS);
    xTaskCreate(task2, "task2", 2048, NULL, 1, NULL);
}