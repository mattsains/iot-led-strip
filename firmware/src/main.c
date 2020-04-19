#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "string.h"
#include "driver/ledc.h"

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("wifi", "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI("wifi", "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("wifi", "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifiConnect(void)
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "SSID here",
            .password = "passphrase here"}};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("wifi", "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI("wifi", "connected to ap");
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI("wifi", "Failed to connect");
    }
    else
    {
        ESP_LOGE("wifi", "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

static ledc_channel_config_t ledc_channel[4];

esp_err_t http_event(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI("http", "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_DATA:
        //ESP_LOGI("http", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            char *result = evt->data;
            result[evt->data_len] = 0;

            char *ptr = strtok(result, ",");
            for (int i = 0; i < 4; i++)
            {
                int level = atoi(ptr);
                ptr = strtok(NULL, ",");

                printf("%i - ", level);
                if (i == 3)
                {
                    printf("\n");
                }

                ledc_set_fade_with_time(ledc_channel[i].speed_mode, ledc_channel[i].channel, level, 500);
                ledc_fade_start(ledc_channel[i].speed_mode, ledc_channel[i].channel, LEDC_FADE_NO_WAIT);
            }
        }

    default:

        break;
    }
    return ESP_OK;
}

void setup_pwm(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel[0] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = 4,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};
    ledc_channel[1] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = 18,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[2] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_2,
        .duty = 0,
        .gpio_num = 16,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[3] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_3,
        .duty = 0,
        .gpio_num = 17,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel_config(&ledc_channel[0]);
    ledc_channel_config(&ledc_channel[1]);
    ledc_channel_config(&ledc_channel[2]);
    ledc_channel_config(&ledc_channel[3]);

    ledc_fade_func_install(0);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    setup_pwm();

    ESP_LOGI("wifi", "ESP_WIFI_MODE_STA");
    wifiConnect();

    esp_http_client_config_t config = {
        .url = "http://192.168.50.199:8080/t",
        .event_handler = http_event};

    while (1)
    {
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_perform(client);

        vTaskDelay(500 / portTICK_RATE_MS);
        esp_http_client_cleanup(client);
    }
}