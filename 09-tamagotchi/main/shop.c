#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

typedef struct ShopItem {
  int id;
  const char *name;
  const char *type;
  const char *slot;
  const char *rarity;
  int price;
  int stock;
} ShopItem;

const char *starts_at;
const char *ends_at;
ShopItem shop[9];

void decode_item(const t_json_value *body, ShopItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "price"), &out->price);
  json_value_int(json_object_get(body, "stock"), &out->stock);

  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
  out->rarity = json_value_string(json_object_get(body, "rarity"));
}

void shop_get() {
  Response *res = client_send_get(http_client, "/shop");
  t_json_value *body = json_parse_buffer(res->body);
  const t_json_value *items = json_object_get(body, "items");
  starts_at = json_value_string(json_object_get(body, "startsAt"));
  ends_at = json_value_string(json_object_get(body, "endsAt"));

  for (int i = 0; i < json_array_size(items); i++) {
    const t_json_value *item = json_array_get(items, i);
    decode_item(item, &shop[i]);
  };
}

void shop_init() {
  http_client = client_create(SERVER_URL);
  shop_get();
}
