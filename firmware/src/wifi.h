#include "esp_wifi.h"
#include "esp_websocket_client.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

typedef void (*_websocket_callback)(int32_t event_type, char *data);

esp_err_t wifi_connect(wifi_sta_config_t config);
void establish_websocket(char *url, _websocket_callback callback);