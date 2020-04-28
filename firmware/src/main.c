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

    err = nvs_get_str(nvs_handle, "ssid", (char *)config->ssid, &required_size);
    if (err)
        return err;

    required_size = 0;
    err = nvs_get_str(nvs_handle, "password", NULL, &required_size);
    if (err)
        return err;
    err = nvs_get_str(nvs_handle, "password", (char *)config->password, &required_size);
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

        // TODO: move this to a helper function.
        char ssid[32];
        char password[64];
        char apiKey[64];

        char acc[65];
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

        ESP_LOGI("httpd", "Received: %s", req_body);
        ESP_LOGI("httpd", "Received SSID: '%s'", ssid);
        ESP_LOGI("httpd", "Received password: '%s'", password);
        ESP_LOGI("httpd", "Received api key: '%s'", apiKey);
    }
    free(req_body);

    const char *thanks = "<h1>Thanks!</h1>";
    httpd_resp_send(req, thanks, strlen(thanks));
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
    setup_nvr();
    setup_leds();

    wifi_ap_start(routes, sizeof(routes) / sizeof(routes[0]));

    // wifi_sta_config_t wifiConfig = {};
    // ESP_ERROR_CHECK(get_wifi_info(&wifiConfig));
    // ESP_ERROR_CHECK(wifi_connect(wifiConfig));

    // establish_websocket("ws://192.168.50.199:8080", websocket_callback);

    while (1)
    {
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}