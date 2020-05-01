#include "wifi.h"
#include "led.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "string.h"

#include "statics.h"

#define WIFI_NS "wifi-setup"
#define WIFI_SSID "ssid"
#define WIFI_PASS "password"
#define API_KEY "apikey"

typedef struct connection_config_t {
    char ssid[32];
    char password[64];
    char apiKey[64];
    char *sslCert;
} connection_config_t;