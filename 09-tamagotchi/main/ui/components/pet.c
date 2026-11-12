#include "pet.h"

extern OLED oled;

static const uint8_t duck_bitmap[DUCK_HEIGHT * 2] = {
    0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFE, 0xF9,
    0x9F, 0xF9, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0x7F, 0xF8,
    0x7F, 0xF8, 0x3F, 0xF0, 0x1F, 0xE0, 0x0C, 0x60, 0x00, 0x00,
};

void pet_sprite_render() {
  State state = state_get();
  if (state.is_dead) {
    oled_draw_bitmap(&oled, (oled.width - DUCK_WIDTH) / 2,
                     (oled.height - DUCK_HEIGHT) / 2, DUCK_WIDTH, DUCK_HEIGHT,
                     false, duck_bitmap);
    return;
  }

  oled_draw_bitmap(&oled, 8, 26, DUCK_WIDTH, DUCK_HEIGHT, false, duck_bitmap);
}
