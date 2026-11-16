#include "pet.h"
#include "animation.h"
#include "assets/animations.h"
#include "oled.h"

extern OLED oled;

void pet_sprite_render() {
  Animation *idle = animations_cat_idle();
  //  State state = state_get();
  oled_draw_bitmap(&oled, 0, 10, idle->width, idle->height, false,
                   animation_frame(idle));
}
