#include "./env.h"
#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "inbox.c"
#include "oled.h"
#include "rotary_encoder.h"
#include "stopwatch.c"
#include "wifi.h"

PassiveBuzzer buzzer;
int wholenote = (60000 * 4) / 140;
OLED oled;
RotaryEncoder rotary_encoder;

Option options[2] = {{.key = "Inbox", .value = 0},
                     {.key = "Stopwatch", .value = 1}};

Menu menu = {
    .current = 0,
    .length = 2,
    .selected = -1,
};

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

  menu.options = options;
  menu.buzzer = &buzzer;
  buzzer_play_melody(buzzer, wholenote, sound_boot());
  wifi_connect();
}

void app_main(void) {
  init();
  inbox_on_boot();

  while (1) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&menu);

    if (rotary_encoder_is_pressed(&rotary_encoder)) {
      rotary_encoder_wait_for_release(&rotary_encoder);
      menu_select(&menu);
    }

    switch (menu.selected) {
    case 0:
      inbox_update();
      break;
    case 1:
      stopwatch_on_start();
      stopwatch_update();
      break;
    default:
      oled_display_menu(&oled, menu);
      oled_update(&oled);
      break;
    }
    menu.selected = -1;
  };
};
