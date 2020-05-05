#include "main.h"

/**
 * This method returns true if the digital LED line is pulled low.
 */
bool isResetHeld()
{
    const char ioPin = 12;
    gpio_config_t resetConfig;
    resetConfig.intr_type = GPIO_PIN_INTR_DISABLE;
    resetConfig.mode = GPIO_MODE_INPUT;
    resetConfig.pin_bit_mask = 1ULL << ioPin; // DAT pin
    resetConfig.pull_down_en = 0;
    resetConfig.pull_up_en = 1;
    gpio_config(&resetConfig);

    return gpio_get_level(ioPin) == 0;
}

void websocket_callback(int32_t event_type, char *result)
{
    char *ptr = strtok(result, ",");
    for (int i = 0; i < 8; i++)
    {
        char *endptr;
        int level = strtol(ptr, &endptr, 10);
        if (endptr == NULL) {
            ESP_LOGE("main", "Received invalid message.");
        }

        ptr = strtok(NULL, ",");

        printf("%i - ", level);
        if (i == 7)
        {
            printf("\n");
        }

        set_led(i, level);
    }
}

connection_config_t get_connection_info()
{
    connection_config_t result;

    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(WIFI_NS, NVS_READWRITE, &nvs_handle));

    size_t required_size = 0;
    
    esp_err_t err = nvs_get_str(nvs_handle, WIFI_SSID, NULL, &required_size);
    if (err)
        return (connection_config_t){ };

    
    err = nvs_get_str(nvs_handle, WIFI_SSID, (char*) &result.ssid, &required_size);
    if (err)
        return (connection_config_t){ };

    required_size = 0;
    err = nvs_get_str(nvs_handle, WIFI_PASS, NULL, &required_size);
    if (err)
        return (connection_config_t){ };
    err = nvs_get_str(nvs_handle, WIFI_PASS, (char*) &result.password, &required_size);
    if (err)
        return (connection_config_t){ };

    required_size = 0;
    err = nvs_get_str(nvs_handle, API_KEY, NULL, &required_size);
    if (err)
        return (connection_config_t){ };
    err = nvs_get_str(nvs_handle, API_KEY, (char*) &result.apiKey, &required_size);
    if (err)
        return (connection_config_t){ };

    nvs_close(nvs_handle);
    return result;
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

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_send(req, html_setup, strlen(html_setup));
    return ESP_OK;
}

static esp_err_t setup_handler(httpd_req_t *req)
{
    char *req_body = malloc(req->content_len + 1);
    {
        if (!httpd_req_recv(req, req_body, req->content_len))
        {
            httpd_resp_send_408(req);
            return ESP_OK;
        }

        req_body[req->content_len] = 0;

        ESP_LOGI("httpd", "Received something");

        // TODO: move this to a helper function.
        char ssid[32];
        char password[64];
        char apiKey[64];
        char *cert = malloc(req->content_len);

        char *acc = malloc(req->content_len);
        size_t acc_pos = 0;
        char c = req_body[0];
        char *fieldUpdating = NULL;

        for (size_t pos = 0; req_body[pos]; c = req_body[++pos])
        {
            if (c == '+')
                acc[acc_pos++] = ' ';
            else if (c == '=')
            {
                acc[acc_pos] = 0;
                acc_pos = 0;
                if (!strcmp(acc, "ssid"))
                    fieldUpdating = ssid;
                else if (!strcmp(acc, "password"))
                    fieldUpdating = password;
                else if (!strcmp(acc, "apikey"))
                    fieldUpdating = apiKey;
                else if (!strcmp(acc, "cert"))
                    fieldUpdating = cert;
                else
                    fieldUpdating = NULL;
            }
            else if (c == '%')
            {
                char num_c[3];
                num_c[0] = req_body[++pos];
                num_c[1] = req_body[++pos];
                num_c[2] = 0;
                acc[acc_pos++] = strtol(num_c, NULL, 16);
            }
            else if (c == '&')
            {
                acc[acc_pos] = 0;
                acc_pos = 0;
                if (fieldUpdating != NULL)
                {
                    strcpy(fieldUpdating, acc);
                }
            }
            else
                acc[acc_pos++] = c;
        }
        acc[acc_pos] = 0;
        acc_pos = 0;
        if (fieldUpdating != NULL)
        {
            strcpy(fieldUpdating, acc);
        }
        free(acc);
        free(cert);

        //ESP_LOGI("httpd", "Received: %s", req_body);
        ESP_LOGI("httpd", "Received SSID: '%s'", ssid);
        ESP_LOGI("httpd", "Received password: '%s'", password);
        ESP_LOGI("httpd", "Received api key: '%s'", apiKey);
        //ESP_LOGI("httpd", "Received cert: '%s'", cert);

        nvs_handle_t nvs_handle;
        ESP_ERROR_CHECK(nvs_open(WIFI_NS, NVS_READWRITE, &nvs_handle));

        nvs_set_str(nvs_handle, WIFI_SSID, ssid);
        nvs_set_str(nvs_handle, WIFI_PASS, password);
        nvs_set_str(nvs_handle, API_KEY, apiKey);
        nvs_set_str(nvs_handle, CERT, cert);
        nvs_commit(nvs_handle);
    }
    free(req_body);

    const char *thanks = "<h1>Thanks!</h1>";
    httpd_resp_send(req, thanks, strlen(thanks));

    esp_restart();
    return ESP_OK;
}

static httpd_uri_t routes[2] = {{
                                    .uri = "/",
                                    .method = HTTP_GET,
                                    .handler = index_handler,
                                },
                                {
                                    .uri = "/setup",
                                    .method = HTTP_POST,
                                    .handler = setup_handler,
                                }};

void app_main(void)
{
    if (isResetHeld())
    {
        // erase the nvr so we can re-configure the device
        ESP_LOGI("nvs", "Erasing the nvs because reset is low");
        ESP_ERROR_CHECK(nvs_flash_erase());
    }

    setup_nvr();
    setup_leds();

    connection_config_t load_config_result = get_connection_info();

    if (load_config_result.ssid[0] == 0)
    {
        // if there's no wifi configuration, assume that it's the first run of this device, and enter setup mode.
        wifi_ap_start(routes, sizeof(routes) / sizeof(routes[0]));
    }
    else
    {
        wifi_sta_config_t wifiConfig = {
            .ssid = "",
            .password = ""};
        strcpy((char *)wifiConfig.ssid, load_config_result.ssid);
        strcpy((char *)wifiConfig.password, load_config_result.password);
        ESP_ERROR_CHECK(wifi_connect(wifiConfig));

        establish_websocket("wss://iot.sainsbury.io:443/ws", load_config_result.apiKey, load_config_result.sslCert, websocket_callback);
    }

    while (1)
    {
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}