#include <math.h>
#include <stdbool.h>
#include <stdint.h>

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

void state_display() {
  const uint8_t line_height = oled.font.height + 2;
  const uint8_t stats_x = 65;
  const uint8_t bar_x = 78;
  const uint8_t bar_width = 45;
  const uint8_t bar_height = 6;
  const uint8_t stats_y = 14;

  const char *name = state.name ? state.name : "Anonymus";
  if (strcmp(state.status, "sleeping") == 0)
    oled_write_text(&oled, 8, 18, false, "zzzz");

  oled_write_text(&oled, 0, 0, false, "%s", name);

  if (state.is_dead) {
    oled_draw_bitmap(&oled, (oled.width - DUCK_WIDTH) / 2,
                     (oled.height - DUCK_HEIGHT) / 2, DUCK_WIDTH, DUCK_HEIGHT,
                     duck_bitmap);
    oled_write_text(&oled, 0, 52, false, "Pet has died");
    oled_update(&oled);
    return;
  }

  oled_draw_bitmap(&oled, 8, 26, DUCK_WIDTH, DUCK_HEIGHT, duck_bitmap);

  oled_write_text(&oled, stats_x, stats_y, false, "E");
  oled_draw_progress_bar(&oled, bar_x, stats_y + 1, bar_width, bar_height,
                         state.energy);

  oled_write_text(&oled, stats_x, stats_y + line_height, false, "H");
  oled_draw_progress_bar(&oled, bar_x, stats_y + line_height + 1, bar_width,
                         bar_height, state.hunger);

  oled_write_text(&oled, stats_x, stats_y + line_height * 2, false, "A");
  oled_draw_progress_bar(&oled, bar_x, stats_y + line_height * 2 + 1, bar_width,
                         bar_height, state.hydration);

  oled_write_text(&oled, stats_x, stats_y + line_height * 3, false, "L");
  oled_draw_progress_bar(&oled, bar_x, stats_y + line_height * 3 + 1, bar_width,
                         bar_height, state.cleanliness);

  oled_write_text(&oled, stats_x, stats_y + line_height * 4, false, "F");
  oled_draw_progress_bar(&oled, bar_x, stats_y + line_height * 4 + 1, bar_width,
                         bar_height, state.happiness);
}

void state_init() { mqtt_sub(mqtt_client, "/state", update_state); }
