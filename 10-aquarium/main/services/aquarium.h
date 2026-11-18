#pragma once

#include "common/random.h"
#include "common/vector.h"
#include "oled.h"

#include <stdint.h>

#define MAX_FISH 10

typedef enum {
  FISH_GUPPY,
  FISH_GOLDFISH,
  FISH_NEON_TETRA,
  FISH_BETTA,
  FISH_SHARK,
} FishSpecies;
typedef enum { FISH_IDLE } FishStatus;

typedef struct {
  double hunger;
  double energy;
  Vector position;
  Vector target;
  Vector direction;
  FishSpecies species;
  FishStatus status;
} Fish;

Fish *aquarium_get();
int aquarium_size();

void aquarium_add(FishSpecies species);
