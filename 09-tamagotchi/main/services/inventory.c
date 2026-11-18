#include "inventory.h"
#include "services/item.h"

extern HttpClient *http_client;

static InventoryItem equippables[MAX_INVENTORY_SIZE];
static int equippables_size = 0;

static InventoryItem consumables[MAX_INVENTORY_SIZE];
static int consumables_size = 0;

static InventoryItem seeds[MAX_INVENTORY_SIZE];
static int seeds_size = 0;

InventoryItem *inventory_get(ItemType type) {
  switch (type) {
  case ITEM_EQUIPPABLE:
    return equippables;
  case ITEM_CONSUMABLE:
    return consumables;
  case ITEM_SEED:
    return seeds;
  };

  return NULL;
};

int inventory_get_size(ItemType type) {
  switch (type) {
  case ITEM_EQUIPPABLE:
    return equippables_size;
  case ITEM_CONSUMABLE:
    return consumables_size;
  case ITEM_SEED:
    return seeds_size;
  }
  return 0;
}

static void decode_inventory_item(const t_json_value *body,
                                  InventoryItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "quantity"), &out->quantity);
  out->name = json_value_string(json_object_get(body, "name"));
  const char *type = json_value_string(json_object_get(body, "type"));
  const char *slot = json_value_string(json_object_get(body, "slot"));
  out->code = json_value_string(json_object_get(body, "code"));
  out->type = item_type(type);
  out->slot = item_slot(slot);
}

static t_json_value *inventory_body;
static void inventory_refresh_task(void *arg) {
  Response *res = client_send_get(http_client, "/inventory");
  if (res == NULL || res->body == NULL)
    return;

  t_json_value *body = json_parse_buffer(res->body);
  response_destroy(res);

  if (body == NULL)
    return;

  if (inventory_body)
    json_value_destroy(inventory_body);
  inventory_body = body;

  equippables_size = 0;
  consumables_size = 0;

  int count = json_array_size(body);
  for (int i = 0; i < count; i++) {
    InventoryItem item = {0};
    decode_inventory_item(json_array_get(body, i), &item);
    if (item.quantity <= 0)
      continue;

    if (item.type == ITEM_EQUIPPABLE) {
      if (equippables_size < MAX_INVENTORY_SIZE)
        equippables[equippables_size++] = item;
    } else if (item.type == ITEM_CONSUMABLE) {
      if (consumables_size < MAX_INVENTORY_SIZE)
        consumables[consumables_size++] = item;
    } else if (item.type == ITEM_SEED) {
      if (seeds_size < MAX_INVENTORY_SIZE)
        seeds[seeds_size++] = item;
    }
  }
  return;
}

void inventory_refresh_async(void) {
  task_start("inventory_refresh", inventory_refresh_task, NULL);
}

static int inventory_find(ItemType type, int32_t item_id) {
  InventoryItem *items = inventory_get(type);
  int size = inventory_get_size(type);

  for (int i = 0; i < size; i++) {
    if (items[i].id == item_id)
      return i;
  }

  return -1;
}

static void inventory_remove(ItemType type, int index) {
  InventoryItem *items = inventory_get(type);
  int size = inventory_get_size(type);
  if (index < 0 || index >= size)
    return;

  for (int i = index; i < size - 1; i++)
    items[i] = items[i + 1];
  if (type == ITEM_EQUIPPABLE)
    equippables_size--;
  if (type == ITEM_CONSUMABLE)
    consumables_size--;
  if (type == ITEM_SEED)
    seeds_size--;
}

void inventory_use(ItemType type, int32_t item_id) {
  if (type == ITEM_SEED)
    return;

  InventoryItem *items = inventory_get(type);
  int index = inventory_find(type, item_id);

  if (index < 0)
    return;

  InventoryItem *item = &items[index];

  if (type == ITEM_EQUIPPABLE) {
    action_equip(item_id);
    inventory_remove(type, index);
  } else if (type == ITEM_CONSUMABLE) {
    action_consume(item_id);
    item->quantity--;
    if (item->quantity <= 0)
      inventory_remove(type, index);
  }
  inventory_refresh_async();
}
