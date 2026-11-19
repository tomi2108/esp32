#pragma once

#include <string.h>

typedef enum {
  ITEM_TYPE_NONE,
  ITEM_TYPE_EQUIPPABLE,
  ITEM_TYPE_CONSUMABLE,
  ITEM_TYPE_SEED,
} ItemType;

typedef enum {
  ITEM_SLOT_NONE,
  ITEM_SLOT_HEAD,
  ITEM_SLOT_BODY,
  ITEM_SLOT_ACCESSORY,
} ItemSlot;

typedef enum {
  ITEM_RARITY_NONE,
  ITEM_RARITY_COMMON,
  ITEM_RARITY_UNCOMMON,
  ITEM_RARITY_RARE,
  ITEM_RARITY_LEGENDARY,
} ItemRarity;

ItemSlot item_slot(const char *slot);
ItemType item_type(const char *type);
ItemRarity item_rarity(const char *rarty);
