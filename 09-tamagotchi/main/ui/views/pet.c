#include "pet.h"

extern OLED oled;

static void render(Screen *self) {
  State state = state_get();

  if (strcmp(state.status, "sleeping") == 0 && !state.is_dead)
    oled_write_text(&oled, 8, 18, false, "zzzz");

  oled_write_text(&oled, 0, 0, false, "%s", state.name);
  pet_sprite_render();

  if (state.is_dead)
    return oled_write_text(&oled, 0, 52, false, "Pet has died");

  wallet_render(60, 0);
  toolbar_render();
  stats_render();
}

static void update(Screen *self) { toolbar_update(); }

static Screen pet_screen = {
    .render = render,
    .update = update,
};

void pet_view(void) { screen_push(pet_screen); }
