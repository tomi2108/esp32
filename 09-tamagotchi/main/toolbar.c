#include "./env.h"
#include "actions.c"
#include "buzzer.h"
#include "client.h"
#include "inbox.c"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define TOOLBAR_LENGTH 3

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Inbox", .value = 0},
    {.key = "Sleep", .value = 1},
    {.key = "Hydrate", .value = 2},
};

void select_toolbar(void *ctx, int32_t value) {
  switch (value) {
  case 0:
    inbox_display();
    break;
  case 1:
    action_sleep();
    break;
  case 2:
    action_hydrate();
    break;
  default:
    break;
  }
}

Menu toolbar = {.buzzer = &buzzer,
                .current = 0,
                .selected = -1,
                .on_select = select_toolbar,
                .length = TOOLBAR_LENGTH,
                .options = toolbar_options};

void toolbar_display() {
  oled_write_text(&oled, 0, oled.height - oled.font.height,
                  toolbar.current == 0, "I");
  oled_write_text(&oled, oled.font.width + 4, oled.height - oled.font.height,
                  toolbar.current == 1, "S");
  oled_write_text(&oled, oled.font.width * 2 + 8,
                  oled.height - oled.font.height, toolbar.current == 2, "H");

  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&toolbar);
  if (direction == ROTARY_CLOCKWISE)
    menu_next(&toolbar);

  if (rotary_encoder_is_pressed(&rotary_encoder)) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    menu_select(&toolbar);
  }
}
