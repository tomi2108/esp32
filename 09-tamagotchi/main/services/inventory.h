#pragma once

#include "client.h"
#include "common/task.h"
#include "json.h"
#include "response.h"
#include "services/actions.h"
#include "services/item.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_INVENTORY_SIZE 75

typedef struct {
  int32_t id;
  const char *name;
  const char *code;
  ItemType type;
  ItemSlot slot;
  int32_t quantity;
} InventoryItem;

void inventory_refresh_async(void);
InventoryItem *inventory_get(ItemType type);
int inventory_get_size(ItemType type);
void inventory_use(ItemType type, int32_t item_id);
