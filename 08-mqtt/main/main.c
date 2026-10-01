#include "buzzer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "mqttt.h"
#include "oled.h"
#include "soc/gpio_num.h"
#include "wifi.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int wholenote = (60000 * 4) / 140;

OLED oled = {0};
PassiveBuzzer buzzer;

char *current_message = NULL;
uint32_t duration = 0;
bool is_timed_message = false;

void init_components() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(BUZZER_PIN);
}

void show_text(char *text) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 0, text);
  oled_update(&oled);
}

void wifi_connect() {
  show_text("Connecting to \"" WIFI_SSID "\"");
  wifi_init(WIFI_SSID, WIFI_PASSWORD);
}

void recieved_message(MqttClient client, MqttMessage message) {
  if (current_message != NULL) {
    free(current_message);
    current_message = NULL;
  }

  current_message = buffer_read_string(message.buffer);
  if (!buffer_is_eof(message.buffer))
    duration = buffer_read_uint32(message.buffer);
  else
    duration = 0;
  is_timed_message = duration > 0;

  mqtt_destroy_message(message);
  buzzer_play_melody(buzzer, wholenote, melody_new_message());
}

void mqtt_connect() {
  show_text("Connecting to mqtt...");
  MqttClient client =
      mqtt_get_client(MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC);
  mqtt_on_data(client, recieved_message);
}

void display_message() {
  oled_clear(&oled);
  if (current_message != NULL)
    oled_write_text(&oled, 0, 0, current_message);

  if (is_timed_message && duration > 0)
    oled_write_text(&oled, 0, oled.height - 10, "%us", duration);

  oled_update(&oled);
}

void app_main(void) {
  init_components();
  wifi_connect();
  mqtt_connect();
  show_text("No messages");

  while (1) {
    if (current_message != NULL) {
      display_message();

      if (is_timed_message) {
        if (duration > 1) {
          duration--;
        } else {
          free(current_message);
          current_message = NULL;
          is_timed_message = false;
          duration = 0;
          show_text("No messages");
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
