#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;

static int wifi_retry_num = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (wifi_retry_num < 5)
        {
            esp_wifi_connect();
            wifi_retry_num++;
            ESP_LOGI("wifi", "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI("wifi", "connect to the AP fail");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("wifi", "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else
    {
        ESP_LOGI("wifi", "Got unexpected event %i", event_id);
    }
}

esp_err_t wifi_connect(wifi_sta_config_t config)
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = config};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));
    vEventGroupDelete(s_wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI("wifi", "connected to ap");
        return ESP_OK;
    }
    else
    {
        ESP_LOGI("wifi", "Failed to connect to ap");
        return ESP_FAIL;
    }
}

static _websocket_callback _callback;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI("web", "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI("web", "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI("web", "WEBSOCKET_EVENT_DATA");
        ESP_LOGI("web", "Received opcode=%d", data->op_code);
        ESP_LOGW("web", "Received=%.*s", data->data_len, (char *)data->data_ptr);

        if (data->op_code != 1)
            break;

        char result[50];
        memcpy(result, data->data_ptr, data->data_len);
        result[data->data_len] = 0;
        _callback(WEBSOCKET_EVENT_DATA, result);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI("web", "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

void establish_websocket(char *url, _websocket_callback callback)
{
    _callback = callback;
    esp_websocket_client_config_t websocket_cfg = {
        .uri = url,
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}