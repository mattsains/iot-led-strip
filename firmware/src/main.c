#include "pair.c"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "string.h"

#include "wifi.c"
#include "led.c"

void callback(int32_t event_type, char *result)
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

void app_main(void)
{
    setup_leds();

    ESP_ERROR_CHECK(wifiConnect("", ""));

    establish_websocket("ws://192.168.50.199:8080", callback);

    while (1)
    {
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}