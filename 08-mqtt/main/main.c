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

void init() {
  oled_init(&oled, 128, 64, sda_pin, scl_pin);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(buzzer_pin);
}

void recieved_message(MqttClient client, MqttMessage message) {
  char *text = message.text;
  oled_clear(&oled);
  oled_write_text(&oled, 0, 0, text);
  oled_update(&oled);
  buzzer_play_melody(buzzer, wholenote, melody_new_message());
}

void app_main(void) {
  init();
  oled_write_text(&oled, 0, 0, "Connecting...");
  oled_update(&oled);

  wifi_init("Telecentro-2661", "");
  MqttClient client = mqtt_get_client("mqtt://192.168.0.37:1883", "test");
  mqtt_on_data(client, recieved_message);

  oled_write_text(&oled, 0, 0, "Connected. Awaiting messages");
  oled_update(&oled);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
