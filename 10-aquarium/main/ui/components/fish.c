#include "fish.h"
#include "common/vector.h"
#include "oled.h"
#include "services/aquarium.h"

extern OLED oled;

void fish_render(Fish fish) {
  switch (fish.species) {
  case FISH_GUPPY:
    oled_fill_rectangle(&oled, fish.position.x, fish.position.y, 4, 4, true);
    break;
  default:
    oled_draw_circle(&oled, fish.position.x, fish.position.y, 0, 4, true);
    break;
  }
}

void fish_update(Fish *fish) {
  Vector dv = vector_sub(fish->target, fish->position);
  float distance = vector_magnitude(dv);
  if (distance < 1.0f) {
    uint16_t t_x = random_range(0, oled.width);
    uint16_t t_y = random_range(0, oled.height);
    fish->target = vector_new(t_x, t_y);
  }

  float speed = 0.5f;
  fish->direction = vector_normal(dv);
  fish->position =
      vector_add(fish->position, vector_scale(fish->direction, speed));
}
