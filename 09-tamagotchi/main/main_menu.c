#include "main_menu.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

Option main_menu_options[MAIN_MENU_LENGTH] = {
    {.key = "Comida", .value = 0},
    {.key = "Ropita", .value = 1},
    {.key = "Mensajitos", .value = 2},
    {.key = "Tienda", .value = 3},
};

void select_main_menu(void *ctx, int32_t value) {
  switch (value) {
  case 0:
    inventory_display(true);
    break;
  case 1:
    inventory_display(false);
    break;
  case 2:
    inbox_display();
    break;
  case 3:
    shop_display();
    break;
  default:
    break;
  }
}

Menu main_menu = {.buzzer = &buzzer,
                  .current = 0,
                  .selected = -1,
                  .on_select = select_main_menu,
                  .length = MAIN_MENU_LENGTH,
                  .options = main_menu_options};

void display_main_menu() {
  const uint8_t h = ICON_HEIGHT;
  const uint8_t w = ICON_WIDTH;
  const uint8_t dx = w + 4;
  const uint8_t dy = w + 4;

  const uint8_t tb_x = 0;
  const uint8_t tb_y = 10;

  Icon icons[MAIN_MENU_LENGTH] = {
      icons_food_restaurant_eating_utensils_fork_knife(),
      icons_hats_sunglasses(),
      icons_software_email_mailbox_unread_new_messages(),
      icons_software_shopping_cart(),
  };

  oled_write_text(&oled, 0, 0, false, "%s",
                  main_menu_options[main_menu.current].key);

  for (int i = 0; i < MAIN_MENU_LENGTH; i++) {
    uint8_t col = i % 6;
    uint8_t row = i / 6;
    uint8_t x = tb_x + col * dx;
    uint8_t y = tb_y + row * dy;
    oled_draw_bitmap(&oled, x, y, w, h, main_menu.current == i, icons[i]);
  }
}

void open_main_menu(void) {
  bool back = false;

  main_menu.current = 0;
  main_menu.selected = -1;

  while (!back) {
    oled_clear(&oled);
    display_main_menu();
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_DOUBLE) {
      back = true;
      continue;
    }

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&main_menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&main_menu);

    if (click == ROTARY_CLICK_SINGLE && main_menu.current >= 0 &&
        main_menu.current < MAIN_MENU_LENGTH) {
      rotary_encoder_wait_for_release(&rotary_encoder);
      menu_select(&main_menu);
    }

    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
