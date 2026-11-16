#include "pet.h"
#include "animation.h"
#include "assets/animations.h"
#include "oled.h"

extern OLED oled;

void pet_sprite_render() {
  State state = state_get();
  Animation *idle = animations_idle(state.species);
  Animation *sleep = animations_sleep(state.species);

  if (strcmp(state.status, "sleeping") == 0) {
    oled_draw_bitmap(&oled, 0, 10, idle->width, idle->height, false,
                     animation_frame(sleep));
  } else {
    oled_draw_bitmap(&oled, 0, 10, idle->width, idle->height, false,
                     animation_frame(idle));
  }
}
