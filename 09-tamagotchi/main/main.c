#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "inventory.h"
#include "melody.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "spinner.h"
#include "state.c"
#include "toolbar.c"
#include "wifi.h"
#include <stdint.h>

extern State state;
extern bool inventory_is_open;
extern const uint8_t certificate[] asm("_binary_mosquitto_ca_crt_start");

PassiveBuzzer buzzer;
int wholenote = (60000 * 4) / 140;
OLED oled;
RotaryEncoder rotary_encoder;
MqttClient mqtt_client;
HttpClient *http_client;

void modules_init() {
  display_init();
  state_init();
  inbox_init();
}

static void init() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(BUZZER_PIN);
  rotary_encoder_init(&rotary_encoder, A_PIN, B_PIN, BUTTON_PIN);
  buzzer_play_melody(buzzer, wholenote, sound_boot());

  spinner_start(&oled);
  wifi_init(WIFI_SSID, WIFI_PASSWORD);

  mqtt_client = mqtt_get_client(MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD,
                                (const char *)certificate);
  http_client = client_create(SERVER_URL);
  modules_init();
  spinner_stop();

  buzzer_play_melody(buzzer, wholenote, sound_success());
}

void app_main(void) {
  init();
  inventory_refresh_async();
  while (1) {
    oled_clear(&oled);
    state_display();
    if (!state.is_dead && !inventory_is_open) {
      toolbar_display();
    }

    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  };
};
