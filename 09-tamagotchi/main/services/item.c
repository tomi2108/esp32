#include "item.h"

ItemSlot item_slot(const char *slot) {
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
  if (strcmp(type, "equippable") == 0)
    return ITEM_EQUIPPABLE;
  else if (strcmp(type, "consumable") == 0)
    return ITEM_CONSUMABLE;
  else if (strcmp(type, "seed") == 0)
    return ITEM_SEED;

  return ITEM_CONSUMABLE;
};

ItemRarity item_rarity(const char *rarity) {
  if (strcmp(rarity, "common") == 0)
    return ITEM_RARITY_COMMON;
  else if (strcmp(rarity, "uncommon") == 0)
    return ITEM_RARITY_UNCOMMON;
  else if (strcmp(rarity, "rare") == 0)
    return ITEM_RARITY_RARE;
  else if (strcmp(rarity, "legendary") == 0)
    return ITEM_RARITY_LEGENDARY;

  return ITEM_RARITY_COMMON;
};
