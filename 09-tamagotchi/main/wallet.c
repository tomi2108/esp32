#include "esp_log.h"
#include "icons.h"
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

  if (!wallet_res || !wallet_res->body) {
    if (wallet_res)
      response_destroy(wallet_res);
    return;
  }

  if (wallet_res->status_code < 200 || wallet_res->status_code >= 300) {
    response_destroy(wallet_res);
    return;
  }

  t_json_value *wallet_body = json_parse_buffer(wallet_res->body);
  if (wallet_body != NULL) {
    const t_json_value *coins = json_object_get(wallet_body, "coins");
    int32_t new_coins;
    if (json_value_int(coins, &new_coins) == 0)
      wallet_coins = new_coins;
    json_value_destroy(wallet_body);
  }
  response_destroy(wallet_res);
}

static void wallet_refresh_task(void *arg) {
  wallet_get();
  vTaskDelete(NULL);
}

void wallet_refresh_async(void) {
  xTaskCreate(wallet_refresh_task, "wallet_refresh", 8192, NULL, 0, NULL);
}

void wallet_display(int x, int y) {
  uint8_t w = ICON_WIDTH;
  uint8_t h = ICON_HEIGHT;
  Icon wallet_icon = icons_rpg_coin_gold_currency_money_gp();
  oled_draw_bitmap(&oled, x, y, w, h, false, wallet_icon);
  char wallet_label[256];
  snprintf(wallet_label, sizeof(wallet_label), "%" PRId32, wallet_coins);
  oled_write_text(&oled, x + w, y + h / 2, false, "%s", wallet_label);
}
