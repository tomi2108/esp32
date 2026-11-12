
#pragma once

#include "json.h"
#include "mqttt.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Equipment {
  // TODO: not correct, but not used yet.
  //  fix when adding equipabble assests to
  //  pet display
  int32_t head;
  int32_t body;
  int32_t accessory;
} Equipment;

typedef struct State {
  char name[32];
  char status[16];
  double energy;
  double hunger;
  double hydration;
  double cleanliness;
  double happiness;
  bool is_dead;
  Equipment equipment;
} State;

void state_init(void);
State state_get(void);
