
#pragma once

#include "json.h"
#include "mqttt.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum { SLOT_HEAD, SLOT_BODY, SLOT_ACCESSORY } EquipmentSlot;

typedef struct EquippedItem {
  int32_t id;
  int32_t price;
  EquipmentSlot slot;
  const char *code;
  const char *name;
  const char *rarity;
  const char *type;
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
  char name[32];
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
