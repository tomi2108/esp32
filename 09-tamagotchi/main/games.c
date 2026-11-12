#include "games.h"
#include "icons.h"
#include "screen.h"
#include "tictactoe.h"
#include "toolbar.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

Option games_options[GAMES_MENU_LENGTH] = {
    {.key = "Ta te ti", .value = 0},
    {.key = "Snake", .value = 1},
};

void select_game(void *ctx, int32_t value) {
  switch (value) {
  case 0:
    tictactoe_play();
    break;
  case 1:
    snake_play();
    break;
  default:
    break;
  }
}

Menu games_menu = {.buzzer = &buzzer,
                   .current = 0,
                   .selected = -1,
                   .on_select = select_game,
                   .length = GAMES_MENU_LENGTH,
                   .options = games_options};

void games_menu_render(Screen *self) {
  const uint8_t h = ICON_HEIGHT;
  const uint8_t w = ICON_WIDTH;
  const uint8_t dx = w + 4;
  const uint8_t dy = w + 4;

  const uint8_t tb_x = 0;
  const uint8_t tb_y = 10;

  Icon icons[GAMES_MENU_LENGTH] = {
      icons_software_signs_maths_multiplication_x_crossout_checkmark_cancel(),
      icons_misc_snake_viper_cobra_poison_1(),
  };

  oled_write_text(&oled, 0, 0, false, "%s",
                  games_options[games_menu.current].key);

  for (int i = 0; i < GAMES_MENU_LENGTH; i++) {
    uint8_t col = i % 6;
    uint8_t row = i / 6;
    uint8_t x = tb_x + col * dx;
    uint8_t y = tb_y + row * dy;
    oled_draw_bitmap(&oled, x, y, w, h, games_menu.current == i, icons[i]);
  }
}

void games_menu_update(Screen *self) {
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);
  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
  }

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&games_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&games_menu);

  if (click == ROTARY_CLICK_SINGLE && games_menu.current >= 0 &&
      games_menu.current < GAMES_MENU_LENGTH) {
    rotary_encoder_wait_for_release(&rotary_encoder);
    menu_select(&games_menu);
  }
}

Screen games_menu_screen = {.render = games_menu_render,
                            .update = games_menu_update};

void open_games_menu(void) {
  games_menu.current = 0;
  games_menu.selected = -1;
  screen_push(games_menu_screen);
};
