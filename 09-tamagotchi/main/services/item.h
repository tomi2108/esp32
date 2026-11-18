#pragma once

#include <string.h>

typedef enum {
  ITEM_EQUIPPABLE,
  ITEM_CONSUMABLE,
  ITEM_SEED,
} ItemType;

typedef enum {
  ITEM_SLOT_NONE,
  ITEM_SLOT_HEAD,
  ITEM_SLOT_BODY,
  ITEM_SLOT_ACCESSORY,
} ItemSlot;

typedef enum {
  ITEM_RARITY_COMMON,
  ITEM_RARITY_UNCOMMON,
  ITEM_RARITY_RARE,
  ITEM_RARITY_LEGENDARY,
} ItemRarity;

ItemSlot item_slot(const char *slot);
ItemType item_type(const char *type);
ItemRarity item_rarity(const char *rarty);
