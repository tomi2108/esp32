#include "inventory.h"

extern OLED oled;
extern PassiveBuzzer buzzer;
extern RotaryEncoder rotary_encoder;

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

static void update_options(bool equippable) {
  InventoryItem *items = inventory_get(equippable);
  int size = inventory_get_size(equippable);
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

static void render(Screen *self) {
  bool equippable = *(bool *)self->context;

  const OLEDMenuHeader inventory_header = {
      .title = equippable ? "Ropita" : "Comida",
      .inverted = true,
      .separator = true,
  };

  if (inventory_get_size(equippable) == 0) {
    oled_write_text(&oled, 0, oled.height / 2 - 3, false, "Inventory empty");
    return;
  }

  OLEDMenu menu_config = {
      .menu = inventory_menu,
      .header = &inventory_header,
      .decorations = inventory_decorations,
      .bitmap_width = ICON_WIDTH,
      .bitmap_height = ICON_HEIGHT,
  };

  oled_display_menu(&oled, &menu_config);
}

static void update(Screen *self) {
  bool equippable = *(bool *)self->context;

  RotaryDirection direction = rotary_encoder_direction(&rotary_encoder);
  RotaryClick click = rotary_encoder_get_click(&rotary_encoder);
  int size = inventory_get_size(equippable);

  if (click == ROTARY_CLICK_DOUBLE) {
    screen_pop();
  }

  if (size == 0)
    return;

  if (direction == ROTARY_CLOCKWISE)
    menu_next(&inventory_menu);

  if (direction == ROTARY_COUNTERCLOCKWISE)
    menu_prev(&inventory_menu);

  if (click == ROTARY_CLICK_SINGLE && inventory_menu.current >= 0 &&
      inventory_menu.current < size) {
    int32_t item_id = inventory_menu.options[inventory_menu.current].value;
    inventory_use(equippable, item_id);
    update_options(equippable);
  }
}

Screen inventory_screen = {
    .render = render,
    .update = update,
};

void inventory_view(bool equippable) {
  update_options(equippable);
  bool *ctx = malloc(sizeof(bool));
  *ctx = equippable;
  inventory_screen.context = ctx;
  inventory_menu.current = 0;
  inventory_menu.selected = -1;
  screen_push(inventory_screen);
}
