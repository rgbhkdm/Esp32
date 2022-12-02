/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_smartconfig.h"
#include "driver/gpio.h"
#include "iot_button.h"

#define LED_PIN 2

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;
esp_netif_t *sta_netif;

static void oneshot_timer_callback(void* arg);

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;


static const char *TAG = "Demo";

static void oneshot_timer_cb(void* arg);
static void button_single_click_cb(void *arg,void *usr_data);
static void button_long_press_cb(void *arg,void *usr_data);
static void smartconfig_example_task(void *parm);
static void initialise_oneshot_timer(void );
static void led_blink_task(void *parm);
static void initialise_button(void);
static void save_wifi_config(wifi_config_t *wifi_config);
static esp_err_t read_wifi_config(wifi_config_t *sta_config);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");
        
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);

        //save_wifi_config(&wifi_config);

        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;

    wifi_config_t myconfig = {0};
 
    ESP_LOGI(TAG, "creat smartconfig_example_task");
    
    // Get the wifi configuration information, and if it is configured, connect to the wifi directly
    esp_wifi_get_config(ESP_IF_WIFI_STA, &myconfig);
    if (strlen((char*)myconfig.sta.ssid) > 0)
    {
        ESP_LOGI(TAG, "alrealy set, SSID is :%s,start connect", myconfig.sta.ssid);
        esp_wifi_connect();
        
    }
    // If it has not been configured, perform the smartconfig operation
    else
    {
        ESP_LOGI(TAG, "have no set, start to config");
        ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );//支持APP ESPTOUCH和微信AIRKISS
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
        
    }

    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, true, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
            
            xTaskCreate(led_blink_task, "led_blink_task", 1024, NULL, 1, NULL);            
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop(); 
        }
        
        vTaskDelete(NULL);
    }
}

static void led_blink_task(void *parm)
{
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    int ON = 0;
    int count = 20;
    while (true)
    {
        ON = !ON;
        gpio_set_level(LED_PIN, ON);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (count <= 0)
        {
            gpio_set_level(LED_PIN, 0);
            vTaskDelete(NULL);
        }
        count--;
    }
}
static void oneshot_timer_cb(void *arg)
{
    initialise_wifi();
    // xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    xTaskCreate(led_blink_task, "led_blink_task", 1024, NULL, 1, NULL);
}
static void button_long_press_cb(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_LONG_PRESS_START");
    //ESP_ERROR_CHECK(nvs_flash_erase());    
    ESP_ERROR_CHECK(esp_wifi_restore());
    
    if(s_wifi_event_group!=NULL)
    {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group=NULL;
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    
    ESP_ERROR_CHECK(esp_event_loop_delete_default());

    esp_netif_destroy_default_wifi(sta_netif);

    ESP_ERROR_CHECK(esp_wifi_stop());
     
    initialise_oneshot_timer();    
}
static void button_single_click_cb(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");    
}

static void initialise_oneshot_timer(void )
{
     const esp_timer_create_args_t oneshot_timer_args = {
            .callback = &oneshot_timer_cb,
            /* name is optional, but may help identify the timer when debugging */
            .name = "one-shot"
    };
    esp_timer_handle_t oneshot_timer;
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));

    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 5000000));
}

static void initialise_button(void)
{
    // create gpio button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = 5000,
        .short_press_time = 50,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_15,
            .active_level = 0,
        },
    };
    button_handle_t btn_handle = iot_button_create(&gpio_btn_cfg);
    if (NULL == btn_handle)
    {
        ESP_LOGE(TAG, "Button create failed");
    }

    
    iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);
    iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_START, button_long_press_cb,NULL);
}

// Save the wifi configuration parameter structure variable wifi_config to nvs
static void save_wifi_config(wifi_config_t *wifi_config)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open("WIFI_CONFIG", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_ERROR_CHECK(err);
    }
    else
    {
        ESP_ERROR_CHECK(nvs_set_blob(my_handle, "wifi_config", wifi_config, sizeof(wifi_config_t)));
        ESP_ERROR_CHECK(nvs_commit(my_handle));
    }
    nvs_close(my_handle);
}

// Read the wifi configuration from nvs to the given sta_config structure variable

static esp_err_t read_wifi_config(wifi_config_t *sta_config)
{
    nvs_handle my_handle;
    esp_err_t err = nvs_open("WIFI_CONFIG", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_ERROR_CHECK(err);
    }
    else
    {
        uint32_t len = sizeof(wifi_config_t);
        esp_err_t err = nvs_get_blob(my_handle, "wifi_config", sta_config, &len);
    }
    nvs_close(my_handle);
    return err;
}

void app_main(void)
{
    wifi_config_t wifi_config_temp;
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    initialise_button();

    initialise_wifi();

    
}
