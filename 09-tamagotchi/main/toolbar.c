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

#define TOOLBAR_LENGTH 4

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Inbox", .value = 0},
    {.key = "Sleep", .value = 1},
    {.key = "Hydrate", .value = 2},
    {.key = "Inventory", .value = 3},
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
  int w = ICON_WIDTH;
  int h = ICON_HEIGHT;
  int dx = w + 4;
  Icon icons[TOOLBAR_LENGTH] = {
      icons_software_email_mailbox_unread_new_messages(),
      icons_rpg_debuff_zz_sleep_status(),
      icons_weather_water_droplet_liquid_rain_element_small(),
      icons_tools_crafting_chest_locked_loot(),
  };
  for (int i = 0; i < TOOLBAR_LENGTH; i++)
    oled_draw_bitmap(&oled, dx * i, y, w, h, toolbar.current == i, icons[i]);

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
};
