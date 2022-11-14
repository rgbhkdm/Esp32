#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_PIN 2

void app_main(void)
{
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    int ON = 0;

    ESP_LOGW("LOG", "This is a warning");
    ESP_LOGI("LOG", "This is an info");
    ESP_LOGD("LOG", "This is a debug");
    ESP_LOGV("LOG", "This is a verbose");

    while (true)
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}