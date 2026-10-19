#include "shop.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;
extern bool hide_toolbar;

const char *starts_at;
const char *ends_at;
ShopItem shop[MAX_SHOP_SIZE];
static int32_t shop_size;

static char shop_labels[MAX_SHOP_SIZE][64];
static char shop_prices[MAX_SHOP_SIZE][12];
static const char *shop_price_ptrs[MAX_SHOP_SIZE];
static const uint8_t *shop_bitmaps[MAX_SHOP_SIZE];

static Option shop_options[MAX_SHOP_SIZE];
static Menu shop_menu = {
    .options = shop_options,
    .length = 0,
    .current = 0,
    .selected = -1,
    .buzzer = &buzzer,
    .context = NULL,
};

void decode_item(const t_json_value *body, ShopItem *out) {
  json_value_int(json_object_get(body, "id"), &out->id);
  json_value_int(json_object_get(body, "price"), &out->price);
  json_value_int(json_object_get(body, "stock"), &out->stock);

  out->name = json_value_string(json_object_get(body, "name"));
  out->type = json_value_string(json_object_get(body, "type"));
  out->slot = json_value_string(json_object_get(body, "slot"));
  out->rarity = json_value_string(json_object_get(body, "rarity"));
}

void shop_get(void) {
  Response *shop_res = client_send_get(http_client, "/shop");
  t_json_value *shop_body = json_parse_buffer(shop_res->body);

  if (!shop_body)
    return;

  const t_json_value *items = json_object_get(shop_body, "items");
  int count = json_array_size(items);

  starts_at = json_value_string(json_object_get(shop_body, "startsAt"));
  ends_at = json_value_string(json_object_get(shop_body, "endsAt"));

  shop_size = 0;

  for (int i = 0; i < count && shop_size < MAX_SHOP_SIZE; i++) {
    const t_json_value *item = json_array_get(items, i);
    ShopItem decoded_item;
    decode_item(item, &decoded_item);
    if (decoded_item.stock <= 0)
      continue;

    int index = shop_size++;
    shop[index] = decoded_item;
    snprintf(shop_labels[index], sizeof(shop_labels[index]), "%s x%" PRId32,
             shop[index].name ? shop[index].name : "Item", shop[index].stock);
    snprintf(shop_prices[index], sizeof(shop_prices[index]), "%" PRId32,
             shop[index].price * shop[index].stock);

    shop_options[index].key = shop_labels[index];
    shop_options[index].value = shop[index].id;
    shop_bitmaps[index] = icons_rpg_coin_gold_currency_money_gp();
    shop_price_ptrs[index] = shop_prices[index];
  }

  shop_menu.length = shop_size;

  if (shop_menu.current >= shop_size)
    shop_menu.current = shop_size > 0 ? shop_size - 1 : 0;
}

static void shop_refresh_task(void *arg) {
  shop_get();
  vTaskDelete(NULL);
}

void shop_refresh_async(void) {
  xTaskCreate(shop_refresh_task, "shop_refresh", 8192, NULL, 0, NULL);
}

static void buy(int32_t item_id) {
  t_json *body = json_create();
  json_object_start(body);
  json_key(body, "itemId");
  json_int(body, item_id);
  json_object_end(body);
  client_send_post(http_client, json_buffer(body), "/shop");
  shop_refresh_async();
  wallet_refresh_async();
}

void display_refresh_time(void) {}

void shop_display(void) {
  hide_toolbar = true;
  bool back = false;

  shop_menu.current = 0;
  shop_menu.selected = -1;

  while (!back) {
    RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
    RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

    if (click == ROTARY_CLICK_DOUBLE) {
      back = true;
      continue;
    }

    if (shop_size == 0) {
      oled_clear(&oled);
      oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Shop empty");
      oled_update(&oled);
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (direction == ROTARY_CLOCKWISE)
      menu_next(&shop_menu);
    if (direction == ROTARY_COUNTERCLOCKWISE)
      menu_prev(&shop_menu);
    if (click == ROTARY_CLICK_SINGLE && shop_menu.current >= 0 &&
        shop_menu.current < shop_size)
      buy(shop_menu.options[shop_menu.current].value);

    oled_clear(&oled);
    oled_display_menu_with_bitmaps(&oled, shop_menu, shop_bitmaps,
                                   shop_price_ptrs, 16, 16);
    display_refresh_time();
    oled_update(&oled);

    vTaskDelay(pdMS_TO_TICKS(10));
  }

  hide_toolbar = false;
}
