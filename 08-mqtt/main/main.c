#include "buzzer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "mqttt.h"
#include "oled.h"
#include "soc/gpio_num.h"
#include "wifi.h"
#include <stdint.h>
#include <string.h>


int wholenote = (60000 * 4) / 140;
gpio_num_t buzzer_pin = GPIO_NUM_27;
gpio_num_t sda_pin = GPIO_NUM_21;
gpio_num_t scl_pin = GPIO_NUM_22;

OLED oled = {0};
PassiveBuzzer buzzer;

void init_components() {
  oled_init(&oled, 128, 64, sda_pin, scl_pin);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(buzzer_pin);
}

void show_text(char *text) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 0, text);
  oled_update(&oled);
}

void wifi_connect() {
  show_text("Connecting to wifi \"" WIFI_SSID "\"");
  wifi_init(WIFI_SSID, WIFI_PASSWORD);
}

void recieved_message(MqttClient client, MqttMessage message) {
  char *text = buffer_read_string(message.buffer);
  mqtt_destroy_message(message);
  show_text(text);
  buzzer_play_melody(buzzer, wholenote, melody_new_message());
}

void mqtt_connect() {
  show_text("Connecting to mqtt...");
  MqttClient client =
      mqtt_get_client(MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC);
  mqtt_on_data(client, recieved_message);
}

void app_main(void) {
  init_components();
  wifi_connect();
  mqtt_connect();
  show_text("No messages");

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
