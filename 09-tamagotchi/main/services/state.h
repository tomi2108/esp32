
#pragma once

#include "json.h"
#include "mqttt.h"
#include "services/item.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct EquippedItem {
  int32_t id;
  int32_t price;
  ItemSlot slot;
  const char *code;
  const char *name;
  ItemRarity rarity;
  ItemType type;
} EquippedItem;

typedef struct Equipment {
  EquippedItem head;
  EquippedItem body;
  EquippedItem accessory;
} Equipment;

typedef enum {
  PET_IDLE,
  PET_SLEEPING,
} PetStatus;

typedef enum {
  PET_CAT,
  PET_DOG,
  PET_RABBIT,
} PetSpecies;

typedef struct State {
  const char *name;
  PetStatus status;
  PetSpecies species;
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
