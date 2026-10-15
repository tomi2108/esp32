#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define MAX_INVENTORY_SIZE 150

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

typedef struct InventoryItem {
  int id;
  const char *name;
  const char *type;
  const char *slot;
  int quantity;
} InventoryItem;

InventoryItem inventory[MAX_INVENTORY_SIZE];
int inventory_size = 0;

void decode_inventory_item(const t_json_value *body, InventoryItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "quantity"), &out->quantity);

  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
}

void inventory_get() {
  Response *res = client_send_get(http_client, "/inventory");
  t_json_value *body = json_parse_buffer(res->body);

  inventory_size = json_array_size(body);
  if (inventory_size > MAX_INVENTORY_SIZE)
    inventory_size = MAX_INVENTORY_SIZE;

  for (int i = 0; i < inventory_size; i++) {
    const t_json_value *item = json_array_get(body, i);
    decode_inventory_item(item, &inventory[i]);
  }
}

void inventory_init() {
  http_client = client_create(SERVER_URL);
  inventory_get();
}
