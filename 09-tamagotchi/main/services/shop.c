#include "shop.h"

extern HttpClient *http_client;

const char *starts_at;
const char *ends_at;

static ShopItem shop[MAX_SHOP_SIZE];
static int shop_size;

static void decode_item(const t_json_value *body, ShopItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "price"), &out->price);
  json_value_int(json_object_get(body, "stock"), &out->stock);
  out->name = json_value_string(json_object_get(body, "name"));
  const char *type = json_value_string(json_object_get(body, "type"));
  const char *slot = json_value_string(json_object_get(body, "slot"));
  const char *rarity = json_value_string(json_object_get(body, "rarity"));

  out->type = item_type(type);
  out->slot = item_slot(slot);
  out->rarity = item_rarity(rarity);
}

static t_json_value *shop_body;
static void shop_refresh_task(void *arg) {
  Response *res = client_send_get(http_client, "/shop");
  if (res == NULL || res->body == NULL)
    return;

  t_json_value *body = json_parse_buffer(res->body);
  response_destroy(res);
  if (body == NULL)
    return;

  if (shop_body)
    json_value_destroy(shop_body);
  shop_body = body;

  shop_size = 0;

  const t_json_value *items = json_object_get(body, "items");
  starts_at = json_value_string(json_object_get(body, "startsAt"));
  ends_at = json_value_string(json_object_get(body, "endsAt"));

  int count = json_array_size(items);
  for (int i = 0; i < count; i++) {
    ShopItem item = {0};
    decode_item(json_array_get(items, i), &item);
    if (item.stock <= 0)
      continue;
    shop[shop_size++] = item;
  }
}

void shop_refresh_async(void) {
  task_start("shop_refresh", shop_refresh_task, NULL);
}

ShopItem *shop_get(void) { return shop; }
int shop_get_size(void) { return shop_size; }

static int shop_find(int32_t item_id) {
  ShopItem *shop = shop_get();
  int size = shop_get_size();

  for (int i = 0; i < size; i++) {
    if (shop[i].id == item_id)
      return i;
  }

  return -1;
}

static void shop_remove(int index) {
  ShopItem *shop = shop_get();
  int size = shop_get_size();
  if (index < 0 || index >= size)
    return;
  for (int i = index; i < size - 1; i++)
    shop[i] = shop[i + 1];
  shop_size--;
}

void shop_buy_task(void *args) {
  int32_t item_id = *(int32_t *)args;

  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "itemId");
  json_int(body, item_id);
  json_object_end(body);

  headers_add(http_client->headers, "Content-Type", "application/json");
  Response *res = client_send_post(http_client, json_buffer(body), "/shop");
  headers_remove(http_client->headers, "Content-Type");

  response_destroy(res);
  json_destroy(body);

  shop_refresh_async();
  wallet_refresh_async();
  inventory_refresh_async();
  free(args);
}

void shop_buy(int32_t item_id) {
  int index = shop_find(item_id);
  if (index < 0)
    return;
  shop_remove(index);
  int32_t *id = malloc(sizeof(int32_t));
  *id = item_id;
  task_start("shop_buy", shop_buy_task, id);
}
