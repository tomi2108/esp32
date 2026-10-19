#include "inventory.h"

extern OLED oled;
extern PassiveBuzzer buzzer;
extern RotaryEncoder rotary_encoder;
extern HttpClient *http_client;

static InventoryItem inventory[MAX_INVENTORY_SIZE];
static int inventory_size;

static Option inventory_options[MAX_INVENTORY_SIZE];
static Menu inventory_menu = {
    .options = inventory_options,
    .length = 0,
    .current = 0,
    .selected = -1,
    .buzzer = &buzzer,
    .context = NULL,
};

static char inventory_labels[MAX_INVENTORY_SIZE][INVENTORY_LABEL_SIZE];

bool inventory_is_open = false;

static void decode_inventory_item(const t_json_value *body,
                                  InventoryItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "quantity"), &out->quantity);
  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
}

static void inventory_update_options(void) {
  for (int i = 0; i < inventory_size; i++) {
    snprintf(inventory_labels[i], sizeof(inventory_labels[i]), "%s x%" PRId32,
             inventory[i].name, inventory[i].quantity);
    inventory_options[i] = (Option){
        .key = inventory_labels[i],
        .value = inventory[i].id,
    };
  }
  inventory_menu.options = inventory_options;
  inventory_menu.length = inventory_size;
  if (inventory_size == 0) {
    inventory_menu.current = 0;
    inventory_menu.selected = -1;
  } else {
    if (inventory_menu.current >= inventory_size)
      inventory_menu.current = inventory_size - 1;
    if (inventory_menu.current < 0)
      inventory_menu.current = 0;
  }
}

bool inventory_fetch(void) {
  Response *res = client_send_get(http_client, "/inventory");
  if (res == NULL || res->body == NULL)
    return false;

  t_json_value *body = json_parse_buffer(res->body);
  if (body == NULL)
    return false;

  inventory_size = 0;

  int count = json_array_size(body);
  if (count > MAX_INVENTORY_SIZE)
    count = MAX_INVENTORY_SIZE;

  for (int i = 0; i < count; i++) {
    InventoryItem item = {0};
    decode_inventory_item(json_array_get(body, i), &item);
    if (item.quantity > 0)
      inventory[inventory_size++] = item;
  }
  inventory_update_options();
  return true;
}

static void inventory_refresh_task(void *arg) {
  inventory_fetch();
  vTaskDelete(NULL);
}

void inventory_refresh_async(void) {
  xTaskCreate(inventory_refresh_task, "inventory_refresh", 8192, NULL, 0, NULL);
}

static int inventory_find(int32_t item_id) {
  for (int i = 0; i < inventory_size; i++) {
    if (inventory[i].id == item_id)
      return i;
  }
  return -1;
}

static void inventory_remove(int index) {
  for (int i = index; i < inventory_size - 1; i++)
    inventory[i] = inventory[i + 1];
  inventory_size--;
}

static void feed(int32_t item_id) {
  // TODO: equip if equippable
  int index = inventory_find(item_id);
  if (index < 0 || inventory[index].quantity <= 0)
    return;

  inventory[index].quantity--;
  if (inventory[index].quantity == 0)
    inventory_remove(index);
  inventory_update_options();
  action_consume(item_id);
  inventory_refresh_async();
}

void inventory_display(void) {
  inventory_is_open = true;
  bool back = false;

  inventory_menu.current = 0;
  inventory_menu.selected = -1;

  while (!back) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_DOUBLE) {
      back = true;
      continue;
    }

    if (inventory_size == 0) {
      oled_clear(&oled);
      oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inventory empty");
      oled_update(&oled);
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    int size = inventory_size;
    if (size > 0) {
      if (direction == ROTARY_CLOCKWISE)
        menu_next(&inventory_menu);
      if (direction == ROTARY_COUNTERCLOCKWISE)
        menu_prev(&inventory_menu);
      if (click == ROTARY_CLICK_SINGLE && inventory_menu.current >= 0 &&
          inventory_menu.current < inventory_size) {
        feed(inventory_menu.options[inventory_menu.current].value);
      }
    }

    oled_clear(&oled);
    oled_display_menu(&oled, inventory_menu);
    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  inventory_is_open = false;
}
