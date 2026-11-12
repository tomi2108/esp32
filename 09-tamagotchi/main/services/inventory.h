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
#define INVENTORY_LABEL_SIZE 64

typedef struct {
  int32_t id;
  const char *name;
  const char *code;
  const char *type;
  const char *slot;
  int32_t quantity;
} InventoryItem;

void inventory_refresh_async(void);
InventoryItem *inventory_get(bool equippable);
int inventory_get_size(bool equippable);
void inventory_use(bool equippable, int32_t item_id);
