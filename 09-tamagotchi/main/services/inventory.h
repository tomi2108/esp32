#pragma once

#include "client.h"
#include "common/task.h"
#include "json.h"
#include "response.h"
#include "services/actions.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_INVENTORY_SIZE 75

typedef enum { INVENTORY_EQUIPPABLE, INVENTORY_CONSUMABLE } InventoryCategory;

typedef struct {
  int32_t id;
  const char *name;
  const char *code;
  const char *type;
  const char *slot;
  int32_t quantity;
} InventoryItem;

void inventory_refresh_async(void);
InventoryItem *inventory_get(InventoryCategory category);
int inventory_get_size(InventoryCategory category);
void inventory_use(InventoryCategory category, int32_t item_id);
