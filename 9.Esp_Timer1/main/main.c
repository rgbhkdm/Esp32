#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define LED_PIN GPIO_NUM_2

esp_timer_handle_t periodic_timer;
EventGroupHandle_t xCreatedEventGroup;
const int LED0 = BIT0;

static void periodic_timer_callback(void *arg);
static void led_task(void *arg);


void app_main(void)
{
    xCreatedEventGroup = xEventGroupCreate();

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"};

    
    esp_timer_create(&periodic_timer_args, &periodic_timer);

    esp_timer_start_periodic(periodic_timer, 500000);


    xTaskCreate( led_task, "Task1", 2048, NULL, 1, NULL );
}

static void periodic_timer_callback(void *arg)
{
    xEventGroupSetBits(
                         xCreatedEventGroup,    // The event group being updated.
                         BIT0);
}

static void led_task(void *arg)
{
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    int ON = 0;

    while (1)
    {
        /* code */
        xEventGroupWaitBits(
            xCreatedEventGroup, // The event group being tested.
            BIT0,              // The bits within the event group to wait for.
            pdTRUE,             // BIT_0 should be cleared before returning.
            pdTRUE,             // Don't wait for both bits, either bit will do.
            portMAX_DELAY);

        ON = !ON;
        gpio_set_level(LED_PIN, ON);       
    }
}
