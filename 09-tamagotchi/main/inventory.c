
#include "inventory.h"

#include <string.h>

extern OLED oled;
extern PassiveBuzzer buzzer;
extern RotaryEncoder rotary_encoder;
extern HttpClient *http_client;

typedef enum { INVENTORY_EQUIPPABLE, INVENTORY_CONSUMABLE } InventoryCategory;

static InventoryItem equippables[MAX_INVENTORY_SIZE];
static int equippables_size = 0;

static InventoryItem consumables[MAX_INVENTORY_SIZE];
static int consumables_size = 0;

static char inventory_labels[MAX_INVENTORY_SIZE][INVENTORY_LABEL_SIZE];
static char inventory_quantities[MAX_INVENTORY_SIZE][12];

static Option inventory_options[MAX_INVENTORY_SIZE];
static OLEDMenuItemDecoration inventory_decorations[MAX_INVENTORY_SIZE];

static Menu inventory_menu = {
    .options = inventory_options,
    .length = 0,
    .current = 0,
    .selected = -1,
    .buzzer = &buzzer,
    .context = NULL,
};

static InventoryItem *inventory_get_items(InventoryCategory category) {
  return category == INVENTORY_EQUIPPABLE ? equippables : consumables;
}

static int *inventory_get_size(InventoryCategory category) {
  return category == INVENTORY_EQUIPPABLE ? &equippables_size
                                          : &consumables_size;
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

static void inventory_update_options(InventoryItem items[], int size) {
  if (size > MAX_INVENTORY_SIZE)
    size = MAX_INVENTORY_SIZE;

  for (int i = 0; i < size; i++) {
    InventoryItem *item = &items[i];

    snprintf(inventory_labels[i], sizeof(inventory_labels[i]), "%s",
             item->name ? item->name : "Item");

    snprintf(inventory_quantities[i], sizeof(inventory_quantities[i]),
             "x%" PRId32, item->quantity);

    inventory_options[i] = (Option){
        .key = inventory_labels[i],
        .value = item->id,
    };

    inventory_decorations[i] = (OLEDMenuItemDecoration){
        .bitmap = get_item_bitmap(item->code),
        .label = inventory_quantities[i],
    };
  }

  inventory_menu.options = inventory_options;
  inventory_menu.length = size;

  if (size == 0) {
    inventory_menu.current = 0;
    inventory_menu.selected = -1;
  } else {
    if (inventory_menu.current >= size)
      inventory_menu.current = size - 1;

    if (inventory_menu.current < 0)
      inventory_menu.current = 0;
  }
}

static bool inventory_fetch(void) {
  Response *res = client_send_get(http_client, "/inventory");

  if (res == NULL || res->body == NULL)
    return false;

  t_json_value *body = json_parse_buffer(res->body);

  if (body == NULL)
    return false;

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
    }
  }

  return true;
}

static void inventory_refresh_task(void *arg) {
  (void)arg;
  inventory_fetch();
  vTaskDelete(NULL);
}

void inventory_refresh_async(void) {
  xTaskCreate(inventory_refresh_task, "inventory_refresh", 8192, NULL, 0, NULL);
}

static int inventory_find(InventoryItem items[], int size, int32_t item_id) {
  for (int i = 0; i < size; i++) {
    if (items[i].id == item_id)
      return i;
  }

  return -1;
}

static void inventory_remove(InventoryItem items[], int *size, int index) {
  if (index < 0 || index >= *size)
    return;

  for (int i = index; i < *size - 1; i++)
    items[i] = items[i + 1];
  (*size)--;
}

static void inventory_use(InventoryCategory category, int32_t item_id) {
  InventoryItem *items = inventory_get_items(category);
  int *size = inventory_get_size(category);

  int index = inventory_find(items, *size, item_id);

  if (index < 0)
    return;

  InventoryItem *item = &items[index];

  if (item->quantity <= 0)
    return;

  if (category == INVENTORY_EQUIPPABLE) {
    action_equip(item_id);

    inventory_remove(items, size, index);
  } else {
    action_consume(item_id);

    item->quantity--;

    if (item->quantity <= 0) {
      inventory_remove(items, size, index);
    }
  }

  inventory_update_options(items, *size);

  inventory_refresh_async();
}

void inventory_display(bool equipabble) {
  InventoryCategory category =
      equipabble ? INVENTORY_EQUIPPABLE : INVENTORY_CONSUMABLE;

  InventoryItem *items = inventory_get_items(category);
  int *size = inventory_get_size(category);

  const OLEDMenuHeader inventory_header = {
      .title = equipabble ? "Ropita" : "Comida",
      .inverted = true,
      .separator = true,
  };

  bool back = false;

  inventory_menu.current = 0;
  inventory_menu.selected = -1;

  while (!back) {
    inventory_update_options(items, *size);

    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);

    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_DOUBLE) {
      back = true;
      continue;
    }

    if (*size == 0) {
      oled_clear(&oled);
      oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inventory empty");
      oled_update(&oled);
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&inventory_menu);

    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&inventory_menu);

    if (click == ROTARY_CLICK_SINGLE && inventory_menu.current >= 0 &&
        inventory_menu.current < *size) {
      int32_t item_id = inventory_menu.options[inventory_menu.current].value;
      inventory_use(category, item_id);
    }

    OLEDMenu menu_config = {
        .menu = inventory_menu,
        .header = &inventory_header,
        .decorations = inventory_decorations,
        .bitmap_width = ICON_WIDTH,
        .bitmap_height = ICON_HEIGHT,
    };

    oled_display_menu(&oled, &menu_config);
    oled_update(&oled);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
