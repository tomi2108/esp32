#include "aquarium.h"

extern OLED oled;

Fish aquarium[MAX_FISH];
int size;

Fish *aquarium_get() { return aquarium; }
int aquarium_size() { return size; }

void aquarium_add(FishSpecies species) {
  uint16_t x = random_range(0, oled.width);
  uint16_t y = random_range(0, oled.height);

  uint16_t t_x = random_range(0, oled.width);
  uint16_t t_y = random_range(0, oled.height);
  Vector position = vector_new(x, y);
  Vector target = vector_new(t_x, t_y);

  Fish fish = {.species = species,
               .direction = vector_new(0, 0),
               .energy = 100,
               .hunger = 100,
               .target = target,
               .position = position,
               .status = FISH_IDLE};
  if (size < MAX_FISH)
    aquarium[size++] = fish;
}
