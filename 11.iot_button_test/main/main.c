#include <stdio.h>
#include "iot_button.h"
#include "esp_log.h"

static void button_single_click_cb(void *arg,void *usr_data);
static const char *TAG = "Button Test: ";

void app_main(void)
{
    // create gpio button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 3000,
        .short_press_time = 50,
        .gpio_button_config = {
            .gpio_num = 15,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
    if (NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Button create failed");
    }

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);
}

static void button_single_click_cb(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
}


