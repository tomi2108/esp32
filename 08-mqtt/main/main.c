#include "./env.h"
#include "buzzer.h"
#include "inbox.c"
#include "oled.h"
#include "rotary_encoder.h"
#include "wifi.h"

int wholenote = (60000 * 4) / 140;
PassiveBuzzer buzzer;
RotaryEncoder rotary_encoder;

void on_next(void *context) {
  buzzer_set_frequency(&buzzer, 550);
  buzzer_tone_ms(buzzer, 100);
}

void on_prev(void *context) {
  buzzer_set_frequency(&buzzer, 393);
  buzzer_tone_ms(buzzer, 100);
}

Menu menu = {
    .on_next = on_next,
    .on_prev = on_prev,
};

OLED oled;

void initial_screen(char *text) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 0, false, text);
  oled_update(&oled);
}

void wifi_connect() {
  initial_screen("Connecting to \"" WIFI_SSID "\"");
  wifi_init(WIFI_SSID, WIFI_PASSWORD);
};

void init() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(BUZZER_PIN);
  rotary_encoder_init(&rotary_encoder, A_PIN, B_PIN, BUTTON_PIN);

  buzzer_play_melody(buzzer, wholenote, sound_boot());
  wifi_connect();
}

void app_main(void) {
  init();
  inbox_init();

  while (1) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&menu);

    if (rotary_encoder_is_pressed(&rotary_encoder))
      menu_select(&menu);

    inbox_update();
  };
};
