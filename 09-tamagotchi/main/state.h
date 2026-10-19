#pragma once
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "icons.h"
#include "json.h"
#include "mqttt.h"
#include "oled.h"

#define DUCK_WIDTH 16
#define DUCK_HEIGHT 16

typedef struct State {
  const char *name;
  double energy;
  double hunger;
  double hydration;
  double cleanliness;
  double happiness;
  bool is_dead;
  const char *status;
} State;

void state_display();
void state_init();
