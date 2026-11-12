#include "pet.h"

extern OLED oled;

static void pet_display(void) {
  State state = state_get();

  if (strcmp(state.status, "sleeping") == 0 && !state.is_dead)
    oled_write_text(&oled, 8, 18, false, "zzzz");

  oled_write_text(&oled, 0, 0, false, "%s", state.name);

  if (state.is_dead) {
    pet_sprite_display(true);
    oled_write_text(&oled, 0, 52, false, "Pet has died");
    return;
  }

  pet_sprite_display(false);
}

static void pet_render(Screen *self) {
  State state = state_get();

  pet_display();
  wallet_display(60, 0);
  toolbar_render();

  if (!state.is_dead)
    pet_stats_display(state);
}

static void pet_update(Screen *self) { toolbar_update(); }

static Screen pet_screen = {
    .render = pet_render,
    .update = pet_update,
};

void state_display(void) { screen_push(pet_screen); }
