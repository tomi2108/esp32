#include "aquarium.h"

extern OLED oled;

static void render(Screen *self) {
  Fish *aquarium = aquarium_get();
  int size = aquarium_size();

  wallet_render(0, 0);

  for (int i = 0; i < size; i++)
    fish_render(aquarium[i]);

  toolbar_render();
}

static void update(Screen *self) {
  Fish *aquarium = aquarium_get();
  int size = aquarium_size();

  for (int i = 0; i < size; i++)
    fish_update(&aquarium[i]);
  toolbar_update();
}

static Screen aquarium_screen = {
    .render = render,
    .update = update,
};

void aquarium_view(void) { screen_push(aquarium_screen); }
