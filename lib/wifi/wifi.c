#include "wifi.h"

void wifi_init(const char *ssid, const char *password) {
  nvs_flash_init();

  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  wifi_config_t config = {0};

  memcpy(config.sta.ssid, ssid, strlen(ssid));
  memcpy(config.sta.password, password, strlen(password));

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &config);
  esp_wifi_start();
  esp_wifi_connect();
}
