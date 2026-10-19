#include "shop.h"
#include "client.h"
#include "json.h"

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
static int shop_size;

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

void shop_get() {
  Response *res = client_send_get(http_client, "/shop");
  t_json_value *body = json_parse_buffer(res->body);
  const t_json_value *items = json_object_get(body, "items");
  starts_at = json_value_string(json_object_get(body, "startsAt"));
  ends_at = json_value_string(json_object_get(body, "endsAt"));

  shop_size = json_array_size(items);
  for (int i = 0; i < shop_size; i++) {
    const t_json_value *item = json_array_get(items, i);
    decode_item(item, &shop[i]);
  };
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
};

void display_coins() {};
void display_refresh_time() {};

void shop_display() {
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

    int size = shop_size;
    if (size > 0) {
      if (direction == ROTARY_CLOCKWISE)
        menu_next(&shop_menu);
      if (direction == ROTARY_COUNTERCLOCKWISE)
        menu_prev(&shop_menu);
      if (click == ROTARY_CLICK_SINGLE && shop_menu.current >= 0 &&
          shop_menu.current < shop_size) {
        buy(shop_menu.options[shop_menu.current].value);
      }
    }

    oled_clear(&oled);
    oled_display_menu(&oled, shop_menu);
    display_coins();
    display_refresh_time();
    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  hide_toolbar = false;
}
