#include "shop.h"

extern HttpClient *http_client;

static void shop_buy_item(int32_t item_id) {
  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "itemId");
  json_int(body, item_id);
  json_object_end(body);

  Headers headers = headers_create();
  headers_add(headers, "Content-Type", "application/json");
  http_client->headers = headers;
  client_send_post(http_client, json_buffer(body), "/shop");
  headers_destroy(headers);
  json_destroy(body);
}
#include "shop.h"

extern HttpClient *http_client;

const char *starts_at;
const char *ends_at;

static ShopItem shop[MAX_SHOP_SIZE];
static int32_t shop_size;

static void decode_item(const t_json_value *body, ShopItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "price"), &out->price);
  json_value_int(json_object_get(body, "stock"), &out->stock);

  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
  out->rarity = json_value_string(json_object_get(body, "rarity"));
}

static void shop_refresh_task(void *arg) {
  Response *shop_res = client_send_get(http_client, "/shop");

  if (!shop_res || !shop_res->body)
    return;

  t_json_value *shop_body = json_parse_buffer(shop_res->body);

  if (!shop_body)
    return;

  const t_json_value *items = json_object_get(shop_body, "items");

  if (!items) {
    json_value_destroy(shop_body);
    return;
  }

  int count = json_array_size(items);

  starts_at = json_value_string(json_object_get(shop_body, "startsAt"));
  ends_at = json_value_string(json_object_get(shop_body, "endsAt"));

  shop_size = 0;

  for (int i = 0; i < count && shop_size < MAX_SHOP_SIZE; i++) {
    const t_json_value *item = json_array_get(items, i);

    if (!item)
      continue;

    ShopItem decoded_item = {0};
    decode_item(item, &decoded_item);

    if (decoded_item.stock <= 0)
      continue;

    size_t index = shop_size++;
    shop[index] = decoded_item;
  }

  json_value_destroy(shop_body);
}

void shop_refresh_async(void) {
  task_start("shop_refresh", shop_refresh_task, NULL);
}

ShopItem *shop_get(void) { return shop; }

int32_t shop_get_size(void) { return shop_size; }

void shop_buy(int32_t item_id) {
  shop_buy_item(item_id);
  shop_refresh_async();
  wallet_refresh_async();
}
