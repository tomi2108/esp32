#include "item.h"

ItemSlot item_slot(const char *slot) {
  if (slot == NULL)
    return ITEM_SLOT_NONE;

  if (strcmp(slot, "body") == 0)
    return ITEM_SLOT_BODY;
  else if (strcmp(slot, "head") == 0)
    return ITEM_SLOT_HEAD;
  else if (strcmp(slot, "accessory") == 0)
    return ITEM_SLOT_ACCESSORY;
  else
    return ITEM_SLOT_NONE;
}

ItemType item_type(const char *type) {
  if (type == NULL)
    return ITEM_TYPE_NONE;

  if (strcmp(type, "equippable") == 0)
    return ITEM_TYPE_EQUIPPABLE;
  else if (strcmp(type, "consumable") == 0)
    return ITEM_TYPE_CONSUMABLE;
  else if (strcmp(type, "seed") == 0)
    return ITEM_TYPE_SEED;

  return ITEM_TYPE_NONE;
};

ItemRarity item_rarity(const char *rarity) {
  if (rarity == NULL)
    return ITEM_RARITY_NONE;

  if (strcmp(rarity, "common") == 0)
    return ITEM_RARITY_COMMON;
  else if (strcmp(rarity, "uncommon") == 0)
    return ITEM_RARITY_UNCOMMON;
  else if (strcmp(rarity, "rare") == 0)
    return ITEM_RARITY_RARE;
  else if (strcmp(rarity, "legendary") == 0)
    return ITEM_RARITY_LEGENDARY;

  return ITEM_RARITY_NONE;
};
