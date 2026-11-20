#pragma once

#include "client.h"
#include "common/task.h"
#include "env.h"
#include "json.h"
#include "response.h"
#include "services/inventory.h"
#include "services/item.h"
#include "services/wallet.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SHOP_SIZE 50

typedef struct ShopItem {
  int32_t id;
  const char *name;
  ItemType type;
  ItemSlot slot;
  ItemRarity rarity;
  int32_t price;
  int32_t stock;
} ShopItem;

void shop_refresh_async(void);
ShopItem *shop_get(void);
int shop_get_size(void);
void shop_buy(int32_t item_id);
