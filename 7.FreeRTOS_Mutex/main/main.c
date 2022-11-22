#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

xSemaphoreHandle xMutex;

void write_message_console(char *message)
{
    printf(message);
}

void temperature_task(void *pvParameters)
{
    while (1)
    {
        printf("----------------------------------------------------\n");
        printf("Inside temperature_task \n");
        xSemaphoreTake(xMutex, portMAX_DELAY);

        write_message_console("temperature is 35 ℃\n");
        write_message_console("----------------------------------------------------\n");
        xSemaphoreGive(xMutex);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void humidity_task(void *pvParameters)
{
    while (1)
    {
        printf("Inside humidity_task\n");
        xSemaphoreTake(xMutex, portMAX_DELAY);

        write_message_console("humidity is 48%% \n");
        xSemaphoreGive(xMutex);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xMutex = xSemaphoreCreateMutex();
    xTaskCreate(temperature_task, "temperature_task", 2048, NULL, 2, NULL);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    xTaskCreate(humidity_task, "humidity_task", 2048, NULL, 2, NULL);
}