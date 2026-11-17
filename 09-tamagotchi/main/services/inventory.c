#include "inventory.h"

extern HttpClient *http_client;

static InventoryItem equippables[MAX_INVENTORY_SIZE];
static int equippables_size = 0;

static InventoryItem consumables[MAX_INVENTORY_SIZE];
static int consumables_size = 0;

static InventoryItem seeds[MAX_INVENTORY_SIZE];
static int seeds_size = 0;

InventoryItem *inventory_get(InventoryCategory category) {
  switch (category) {
  case INVENTORY_EQUIPPABLE:
    return equippables;
  case INVENTORY_CONSUMABLE:
    return consumables;
  case INVENTORY_SEEDS:
    return seeds;
  };

  return NULL;
};

int inventory_get_size(InventoryCategory category) {
  switch (category) {
  case INVENTORY_EQUIPPABLE:
    return equippables_size;
  case INVENTORY_CONSUMABLE:
    return consumables_size;
  case INVENTORY_SEEDS:
    return seeds_size;
  }
  return 0;
}

static void decode_inventory_item(const t_json_value *body,
                                  InventoryItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "quantity"), &out->quantity);
  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
  out->code = json_value_string(json_object_get(body, "code"));
}

static void inventory_refresh_task(void *arg) {
  Response *res = client_send_get(http_client, "/inventory");
  if (res == NULL || res->body == NULL)
    return;

  t_json_value *body = json_parse_buffer(res->body);
  if (body == NULL)
    return;

  equippables_size = 0;
  consumables_size = 0;

  int count = json_array_size(body);
  for (int i = 0; i < count; i++) {
    InventoryItem item = {0};
    decode_inventory_item(json_array_get(body, i), &item);
    if (item.quantity <= 0 || item.type == NULL)
      continue;

    if (strcmp(item.type, "equippable") == 0) {
      if (equippables_size < MAX_INVENTORY_SIZE)
        equippables[equippables_size++] = item;
    } else if (strcmp(item.type, "consumable") == 0) {
      if (consumables_size < MAX_INVENTORY_SIZE)
        consumables[consumables_size++] = item;
    } else if (strcmp(item.type, "seed") == 0) {
      if (seeds_size < MAX_INVENTORY_SIZE)
        seeds[seeds_size++] = item;
    }
  }
  return;
}

void inventory_refresh_async(void) {
  task_start("inventory_refresh", inventory_refresh_task, NULL);
}

static int inventory_find(InventoryCategory category, int32_t item_id) {
  InventoryItem *items = inventory_get(category);
  int size = inventory_get_size(category);

  for (int i = 0; i < size; i++) {
    if (items[i].id == item_id)
      return i;
  }

  return -1;
}

static void inventory_remove(InventoryCategory category, int index) {
  InventoryItem *items = inventory_get(category);
  int size = inventory_get_size(category);
  if (index < 0 || index >= size)
    return;

  for (int i = index; i < size - 1; i++)
    items[i] = items[i + 1];
  if (category == INVENTORY_EQUIPPABLE)
    equippables_size--;
  if (category == INVENTORY_CONSUMABLE)
    consumables_size--;
  if (category == INVENTORY_SEEDS)
    seeds_size--;
}

void inventory_use(InventoryCategory category, int32_t item_id) {
  if (category == INVENTORY_SEEDS)
    return;

  InventoryItem *items = inventory_get(category);
  int index = inventory_find(category, item_id);

  if (index < 0)
    return;

  InventoryItem *item = &items[index];

  if (category == INVENTORY_EQUIPPABLE) {
    action_equip(item_id);
    inventory_remove(category, index);
  } else if (category == INVENTORY_CONSUMABLE) {
    action_consume(item_id);
    item->quantity--;
    if (item->quantity <= 0)
      inventory_remove(category, index);
  }
  inventory_refresh_async();
}
