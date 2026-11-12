#include "shop.h"

extern PassiveBuzzer buzzer;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

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

static void update_options(void) {
  ShopItem *items = shop_get_items();
  int32_t size = shop_get_size();

  for (int i = 0; i < size; i++) {
    ShopItem *item = &items[i];

    snprintf(shop_labels[i], sizeof(shop_labels[i]), "%s x%" PRId32,
             item->name ? item->name : "Item", item->stock);

    snprintf(shop_prices[i], sizeof(shop_prices[i]), "%" PRId32,
             item->price * item->stock);

    shop_options[i] = (Option){
        .key = shop_labels[i],
        .value = item->id,
    };

    shop_decorations[i] = (OLEDMenuItemDecoration){
        .bitmap = icons_rpg_coin_gold_currency_money_gp(),
        .label = shop_prices[i],
    };
  }

  shop_menu.length = size;

  if (shop_menu.current >= size)
    shop_menu.current = size > 0 ? size - 1 : 0;
}

void refresh_time_render(void) {}

static void render(Screen *self) {
  update_options();

  const OLEDMenu menu_config = {
      .menu = shop_menu,
      .header = &shop_header,
      .decorations = shop_decorations,
      .bitmap_width = 16,
      .bitmap_height = 16,
  };

  if (shop_get_size() == 0) {
    oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Shop empty");
    return;
  }

  OLEDMenu current_menu = menu_config;
  current_menu.menu = shop_menu;

  oled_display_menu(&oled, &current_menu);
  refresh_time_render();
  return;
}

static void update(Screen *self) {
  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);

  if (click == ROTARY_CLICK_DOUBLE)
    screen_pop();

  if (shop_get_size() == 0)
    return;

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&shop_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&shop_menu);

  if (click == ROTARY_CLICK_SINGLE && shop_menu.current >= 0 &&
      shop_menu.current < shop_get_size())
    shop_buy(shop_menu.options[shop_menu.current].value);
}

Screen shop_screen = {
    .render = render,
    .update = update,
};

void shop_view(void) {
  shop_menu.current = 0;
  shop_menu.selected = -1;
  screen_push(shop_screen);
}
