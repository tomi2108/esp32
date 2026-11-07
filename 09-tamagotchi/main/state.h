#pragma once
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "icons.h"
#include "json.h"
#include "mqttt.h"
#include "oled.h"
#include "wallet.h"

#define DUCK_WIDTH 16
#define DUCK_HEIGHT 16

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

void pet_display(void);
void state_stats_display(void);

void state_init(void);
State state_get(void);
