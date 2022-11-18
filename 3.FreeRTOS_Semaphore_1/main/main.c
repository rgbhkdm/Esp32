#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"

SemaphoreHandle_t xSemaphore = NULL;

TaskHandle_t giveTaskHandle = NULL;
TaskHandle_t takeTaskHandle = NULL;

void give_task(void *pvParameters)
{
    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            /* code */
            if(xSemaphoreGive(xSemaphore)==pdTRUE)
            {
                //printf("[%d] Give Ok! [%d] \n", i,xTaskGetTickCount());
                printf("[%d] Give Ok! \n", i);
            }
            else
            {
                //printf("[%d] Give Error! [%d] \n", i,xTaskGetTickCount());
                printf("[%d] Give Error! \n", i);
            }
        }      
        
        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void take_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY)==pdTRUE)
        {
            //printf("Take Ok! [%d] \n", xTaskGetTickCount());
            printf("Take Ok!  \n");            
        }
        else
        {
            //printf("Take Error! [%d] \n", xTaskGetTickCount());
            printf("Take Error!  \n");
        }

        vTaskDelay(1000/ portTICK_RATE_MS);
    }
}

void app_main(void)
{
    xSemaphore = xSemaphoreCreateBinary();
    
    xTaskCreate(give_task, "give_task", 4096, NULL, 10, &giveTaskHandle);
    vTaskDelay(10/ portTICK_RATE_MS);
    //xTaskCreate(take_task, "take_task", 4096, NULL, 2, &takeTaskHandle);
    xTaskCreatePinnedToCore(take_task, "take_task", 4096, NULL, 9, &takeTaskHandle, 1);

    
}