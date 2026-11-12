#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "font.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "inbox.h"
#include "inventory.h"
#include "melody.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "overlay.h"
#include "rotary_encoder.h"
#include "screen.h"
#include "shop.h"
#include "spinner.h"
#include "state.h"
#include "task.h"
#include "toolbar.h"
#include "wallet.h"
#include "wifi.h"
#include <stdint.h>

extern const uint8_t certificate[] asm("_binary_mosquitto_ca_crt_start");

PassiveBuzzer buzzer;
int wholenote = (60000 * 4) / 140;
OLED oled;
RotaryEncoder rotary_encoder;
MqttClient mqtt_client;
HttpClient *http_client;

void modules_init() {
  task_manager_init();
  state_init();
  inbox_init();
}

static void show_progress(uint8_t progress, const char *message) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 12, false, "%s", message);
  oled_draw_progress_bar(&oled, 14, 30, 100, 8, progress, false);
  oled_write_text(&oled, 52, 46, false, "%d%%", progress);
  oled_update(&oled);
}

static void init() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN, OLED_CONTROLLER_SH1106);
  oled.font = font_default();
  show_progress(0, "Iniciando");

  buzzer = passive_buzzer_init(BUZZER_PIN);
  rotary_encoder_init(&rotary_encoder, A_PIN, B_PIN, BUTTON_PIN);
  buzzer_play_melody(buzzer, wholenote, sound_boot());
  show_progress(40, "Conectando a " WIFI_SSID);

  wifi_init(WIFI_SSID, WIFI_PASSWORD);

  mqtt_client = mqtt_get_client(MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD,
                                (const char *)certificate);
  http_client = client_create(SERVER_URL);
  show_progress(70, "Buscando mensajitos");
  modules_init();
  show_progress(100, "");
  buzzer_play_melody(buzzer, wholenote, sound_success());
}

void app_main(void) {
  init();
  wallet_refresh_async();
  shop_refresh_async();
  inventory_refresh_async();
  state_display();

  while (1) {
    oled_clear(&oled);
    Screen *screen = screen_current();
    if (screen->update)
      screen->update(screen);

    overlay_update();

    if (screen->render)
      screen->render(screen);
    overlay_render();

    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  };
};
