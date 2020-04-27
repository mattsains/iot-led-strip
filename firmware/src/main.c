#include "main.h"


void websocket_callback(int32_t event_type, char *result)
{
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

        set_led(i, level);
    }
}

esp_err_t get_wifi_info(wifi_sta_config_t *config)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi-setup", NVS_READWRITE, &nvs_handle));

    size_t required_size = 0;
    esp_err_t err = nvs_get_str(nvs_handle, "ssid", NULL, &required_size);
    if (err)
        return err;

    err = nvs_get_str(nvs_handle, "ssid", (char*)config->ssid, &required_size);
    if (err)
        return err;

    required_size = 0;
    err = nvs_get_str(nvs_handle, "password", NULL, &required_size);
    if (err)
        return err;
    err = nvs_get_str(nvs_handle, "password", (char*)config->password, &required_size);
    if (err)
        return err;

    nvs_close(nvs_handle);
    return ESP_OK;
}

void setup_nvr()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    setup_nvr();
    setup_leds();
    

    wifi_sta_config_t wifiConfig = {};
    ESP_ERROR_CHECK(get_wifi_info(&wifiConfig));
    ESP_ERROR_CHECK(wifi_connect(wifiConfig));

    establish_websocket("ws://192.168.50.199:8080", websocket_callback);

    while (1)
    {
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}