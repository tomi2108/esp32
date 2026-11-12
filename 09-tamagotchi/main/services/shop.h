#pragma once

#include "client.h"
#include "common/task.h"
#include "env.h"
#include "json.h"
#include "response.h"
#include "services/wallet.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_SHOP_SIZE 50

typedef struct ShopItem {
  int32_t id;
  const char *name;
  const char *type;
  const char *slot;
  const char *rarity;
  int32_t price;
  int32_t stock;
} ShopItem;

void shop_refresh_async(void);
ShopItem *shop_get_items(void);
int32_t shop_get_size(void);
void shop_buy(int32_t item_id);
