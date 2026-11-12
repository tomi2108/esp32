#include "toolbar.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Sleep", .value = 0},
    {.key = "Hydrate", .value = 1},
    {.key = "Main Menu", .value = 2},
};

void select_toolbar(void *ctx, int32_t value) {
  switch (value) {
  case 0:
    action_sleep();
    break;
  case 1:
    action_hydrate();
    break;
  case 2:
    open_main_menu();
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

void toolbar_render() {
  const uint8_t h = ICON_HEIGHT;
  const uint8_t w = ICON_WIDTH;
  const uint8_t dx = w + 4;

  const uint8_t tb_x = 60;
  const uint8_t y = ICON_HEIGHT;

  Icon icons[TOOLBAR_LENGTH] = {
      icons_rpg_debuff_zz_sleep_status(),
      icons_weather_water_droplet_liquid_rain_element_small(),
      icons_software_file_folder_directory_explorer()};

  for (int i = 0; i < TOOLBAR_LENGTH; i++) {
    uint8_t x = tb_x + i * dx;
    oled_draw_bitmap(&oled, x, y, w, h, toolbar.current == i, icons[i]);
  }
};

void toolbar_update() {
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
