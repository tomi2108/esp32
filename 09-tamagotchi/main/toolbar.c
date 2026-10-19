#include "toolbar.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Inbox", .value = 0},   {.key = "Sleep", .value = 1},
    {.key = "Hydrate", .value = 2}, {.key = "Inventory", .value = 3},
    {.key = "Shop", .value = 4},
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
  case 4:
    shop_display();
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
  const uint8_t h = ICON_HEIGHT;
  const uint8_t w = ICON_WIDTH;
  const uint8_t dx = w + 4;
  const uint8_t dy = w + 4;

  const uint8_t tb_x = 60;
  const uint8_t tb_y = 0;

  Icon icons[TOOLBAR_LENGTH] = {
      icons_software_email_mailbox_unread_new_messages(),
      icons_rpg_debuff_zz_sleep_status(),
      icons_weather_water_droplet_liquid_rain_element_small(),
      icons_tools_crafting_chest_locked_loot(), icons_software_shopping_cart()};

  for (int i = 0; i < TOOLBAR_LENGTH; i++) {
    uint8_t col = i % 3;
    uint8_t row = i / 3;
    uint8_t x = tb_x + col * dx;
    uint8_t y = tb_y + row * dy;
    oled_draw_bitmap(&oled, x, y, w, h, toolbar.current == i, icons[i]);
  }

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
