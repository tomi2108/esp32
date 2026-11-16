#include "pet.h"
#include "animation.h"
#include "assets/animations.h"
#include "oled.h"

extern OLED oled;

void pet_sprite_render() {
  State state = state_get();
  Animation *animation = animations_pet_status(state.species, state.status);
  oled_draw_bitmap(&oled, 0, 10, animation->width, animation->height, false,
                   animation_frame(animation));
}
