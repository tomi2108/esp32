#include "wifi.h"

static EventGroupHandle_t wifi_events;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    esp_wifi_connect();
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    esp_wifi_connect();
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    xEventGroupSetBits(wifi_events, BIT0);
}

void wifi_init(const char *ssid, const char *password) {
  nvs_flash_init();

  wifi_events = xEventGroupCreate();

  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler,
                             NULL);

  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler,
                             NULL);

  wifi_config_t config = {0};

  strcpy((char *)config.sta.ssid, ssid);
  strcpy((char *)config.sta.password, password);

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &config);
  esp_wifi_start();

  xEventGroupWaitBits(wifi_events, BIT0, pdFALSE, pdTRUE, portMAX_DELAY);
}
