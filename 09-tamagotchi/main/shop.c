#include "shop.h"
#include "toolbar.h"

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;
extern MqttClient mqtt_client;
extern HttpClient *http_client;

const char *starts_at;
const char *ends_at;
ShopItem shop[MAX_SHOP_SIZE];
static int32_t shop_size;

static char shop_labels[MAX_SHOP_SIZE][64];
static char shop_prices[MAX_SHOP_SIZE][12];
static Option shop_options[MAX_SHOP_SIZE];
static OLEDMenuItemDecoration shop_decorations[MAX_SHOP_SIZE];

static Menu shop_menu = {
    .options = shop_options,
    .length = 0,
    .current = 0,
    .selected = -1,
    .buzzer = &buzzer,
    .context = NULL,
};

static const OLEDMenuHeader shop_header = {
    .title = "Tienda",
    .inverted = true,
    .separator = true,
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

    snprintf(shop_labels[index], sizeof(shop_labels[index]), "%s x%" PRId32,
             shop[index].name ? shop[index].name : "Item", shop[index].stock);

    snprintf(shop_prices[index], sizeof(shop_prices[index]), "%" PRId32,
             shop[index].price * shop[index].stock);

    shop_options[index] = (Option){
        .key = shop_labels[index],
        .value = shop[index].id,
    };

    shop_decorations[index] = (OLEDMenuItemDecoration){
        .bitmap = icons_rpg_coin_gold_currency_money_gp(),
        .label = shop_prices[index],
    };
  }

  shop_menu.length = shop_size;

  if (shop_menu.current >= shop_size)
    shop_menu.current = shop_size > 0 ? shop_size - 1 : 0;

  json_value_destroy(shop_body);
}

void shop_refresh_async(void) {
  task_start("shop_refresh", shop_refresh_task, NULL);
}

static void buy(int32_t item_id) {
  api_buy_item(item_id);
  // TODO: optimistically remove
  shop_refresh_async();
  wallet_refresh_async();
}

void display_refresh_time(void) {}

void shop_render(Screen *self) {
  const OLEDMenu menu_config = {
      .menu = shop_menu,
      .header = &shop_header,
      .decorations = shop_decorations,
      .bitmap_width = 16,
      .bitmap_height = 16,
  };

  if (shop_size == 0) {
    oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Shop empty");
    return;
  }

  OLEDMenu current_menu = menu_config;
  current_menu.menu = shop_menu;

  oled_display_menu(&oled, &current_menu);
  display_refresh_time();
  return;
}

void shop_update(Screen *self) {
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_DOUBLE)
    screen_pop();

  if (shop_size == 0)
    return;

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&shop_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&shop_menu);

  if (click == ROTARY_CLICK_SINGLE && shop_menu.current >= 0 &&
      shop_menu.current < shop_size)
    buy(shop_menu.options[shop_menu.current].value);
}

Screen shop_screen = {
    .render = shop_render,
    .update = shop_update,
};

void shop_display(void) {
  shop_menu.current = 0;
  shop_menu.selected = -1;
  screen_push(shop_screen);
};
