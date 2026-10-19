#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "esp_log.h"
#include "icons.h"
#include "inbox.c"
#include "inventory.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define TOOLBAR_LENGTH 2

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Inbox", .value = 0},
    {.key = "Sleep", .value = 1},
    // {.key = "Hydrate", .value = 2},
    // {.key = "Feed", .value = 3},
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
  case 3:
    inventory_display();
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
  int y = oled.height - ICON_HEIGHT;
  oled_draw_bitmap(&oled, 0, y, ICON_WIDTH, ICON_HEIGHT, toolbar.current == 0,
                   icons_email_unread());
  oled_draw_bitmap(&oled, ICON_WIDTH + 4, y, ICON_WIDTH, ICON_HEIGHT,
                   toolbar.current == 1, icons_email_read());
  // oled_write_text(&oled, (oled.font.width + 4) * 2,
  //                 oled.height - oled.font.height, toolbar.current == 2, "H");
  //
  // oled_write_text(&oled, (oled.font.width + 4) * 3,
  //                 oled.height - oled.font.height, toolbar.current == 3, "F");

  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&toolbar);
  if (direction == ROTARY_CLOCKWISE)
    menu_next(&toolbar);

  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);
  if (click == ROTARY_CLICK_SINGLE) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    menu_select(&toolbar);
  }
}
