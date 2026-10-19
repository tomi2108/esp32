#include "esp_log.h"
#include "oled.h"
#include "shop.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;
extern bool hide_toolbar;

int32_t wallet_coins = 0;
void wallet_get(void) {
  Response *wallet_res = client_send_get(http_client, "/wallet");
  t_json_value *wallet_body = json_parse_buffer(wallet_res->body);
  if (!wallet_body)
    return;

  json_value_int(json_object_get(wallet_body, "coins"), &wallet_coins);
}

static void wallet_refresh_task(void *arg) {
  wallet_get();
  vTaskDelete(NULL);
}

void wallet_refresh_async(void) {
  xTaskCreate(wallet_refresh_task, "wallet_refresh", 8192, NULL, 0, NULL);
}
