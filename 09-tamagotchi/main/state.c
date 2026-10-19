#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "icons.h"
#include "json.h"
#include "mqttt.h"
#include "oled.h"

#define DUCK_WIDTH 16
#define DUCK_HEIGHT 16

extern MqttClient mqtt_client;
extern OLED oled;

typedef struct State {
  const char *name;
  double energy;
  double hunger;
  double hydration;
  double cleanliness;
  double happiness;
  bool is_dead;
  const char *status;
} State;

State state;

void update_state(MqttClient client, MqttMessage message) {
  const t_json_value *new_state = json_parse_buffer(message.buffer);
  json_value_double(json_object_get(new_state, "energy"), &state.energy);
  json_value_double(json_object_get(new_state, "hunger"), &state.hunger);
  json_value_double(json_object_get(new_state, "hydration"), &state.hydration);
  json_value_double(json_object_get(new_state, "cleanliness"),
                    &state.cleanliness);
  json_value_double(json_object_get(new_state, "happiness"), &state.happiness);
  json_value_bool(json_object_get(new_state, "is_dead"), &state.is_dead);
  state.status = json_value_string(json_object_get(new_state, "status"));
  state.name = json_value_string(json_object_get(new_state, "name"));
}

static const uint8_t duck_bitmap[DUCK_HEIGHT * 2] = {
    0x07, 0xE0, 0x1F, 0xF8, 0x3F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFE, 0xF9,
    0x9F, 0xF9, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0x7F, 0xF8,
    0x7F, 0xF8, 0x3F, 0xF0, 0x1F, 0xE0, 0x0C, 0x60, 0x00, 0x00,
};
void display_stats(void) {
  const uint8_t w = ICON_WIDTH;
  const uint8_t h = ICON_HEIGHT;
  const uint8_t bar_width = 4;
  const uint8_t gap = 2;
  const uint8_t cell_width = w + bar_width + gap;
  const uint8_t cell_height = h + 2;

  const uint8_t stats_x = 60;
  const uint8_t stats_y = 0;

  Icon icons[5] = {
      icons_software_power_electricity_battery_thunder_lightning_bolt_zap(),
      icons_food_meat_chicken_leg_drumstick(),
      icons_weather_water_droplet_liquid_rain_element_small(),
      icons_cosmetics_bath_shower_1(),
      icons_emoji_face_happy(),
  };

  double stats[5] = {
      state.energy,      state.hunger,    state.hydration,
      state.cleanliness, state.happiness,
  };

  for (int i = 0; i < 5; i++) {
    uint8_t col = i % 3;
    uint8_t row = i / 3;

    uint8_t x = stats_x + col * cell_width;
    uint8_t y = stats_y + row * cell_height;
    uint8_t bar_x = x + w + 1;

    oled_draw_bitmap(&oled, x, y, w, h, false, icons[i]);
    oled_draw_progress_bar(&oled, bar_x, y, bar_width, h, stats[i], true);
  }
}

void state_display() {

  const char *name = state.name ? state.name : "Anonymus";
  if (strcmp(state.status, "sleeping") == 0 && !state.is_dead)
    oled_write_text(&oled, 8, 18, false, "zzzz");

  oled_write_text(&oled, 0, 0, false, "%s", name);

  if (state.is_dead) {
    oled_draw_bitmap(&oled, (oled.width - DUCK_WIDTH) / 2,
                     (oled.height - DUCK_HEIGHT) / 2, DUCK_WIDTH, DUCK_HEIGHT,
                     false, duck_bitmap);
    oled_write_text(&oled, 0, 52, false, "Pet has died");
    return;
  }

  oled_draw_bitmap(&oled, 8, 26, DUCK_WIDTH, DUCK_HEIGHT, false, duck_bitmap);
  display_stats();
}

void state_init() { mqtt_sub(mqtt_client, "/state", update_state); }
