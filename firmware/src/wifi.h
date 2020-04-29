#include "esp_wifi.h"
#include "esp_websocket_client.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

#include <esp_http_server.h>

#include "statics.h"

esp_err_t wifi_connect(wifi_sta_config_t config);
void wifi_ap_start(httpd_uri_t *routes, size_t routes_length);

typedef void (*_websocket_callback)(int32_t event_type, char *data);

void establish_websocket(char *url, char *apikey, _websocket_callback callback);