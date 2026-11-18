#include "toolbar.h"
#include "icons.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

Option toolbar_options[TOOLBAR_LENGTH] = {
    {.key = "Alimentar", .value = 0},
    {.key = "Tienda", .value = 1},
};

void select_toolbar(void *ctx, int32_t value) {
  switch (value) {
  case 0:
    break;
  case 1:
    break;
  case 2:
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

  const uint8_t tb_x = 0;
  const uint8_t y = oled.height - 2 - ICON_HEIGHT;

  Icon icons[TOOLBAR_LENGTH] = {
      icons_food_soda_can_fizzy_drink_1(),
      icons_software_shopping_cart(),
  };

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
