#include "stats.h"
#include "services/state.h"

extern OLED oled;

void stats_render() {
  State state = state_get();
  const uint8_t w = ICON_WIDTH;
  const uint8_t h = ICON_HEIGHT;
  const uint8_t bar_width = 4;
  const uint8_t gap = 2;
  const uint8_t cell_width = w + bar_width + gap;
  int stats_x = oled.width - cell_width * 5 - 5;
  int stats_y = oled.height - h;

  Icon icons[5] = {
      icons_software_power_electricity_battery_thunder_lightning_bolt_zap(),
      icons_food_meat_chicken_leg_drumstick(),
      icons_weather_water_droplet_liquid_rain_element_small(),
      icons_cosmetics_bath_shower_1(),
      icons_emoji_face_smiling_smiley(),
  };

  double stats[5] = {
      state.energy,      state.hunger,    state.hydration,
      state.cleanliness, state.happiness,
  };

  for (int i = 0; i < 5; i++) {
    uint8_t x = stats_x + i * cell_width;
    uint8_t bar_x = x + w + 1;
    oled_draw_bitmap(&oled, x, stats_y, w, h, false, icons[i]);
    oled_draw_progress_bar(&oled, bar_x, stats_y, bar_width, h, stats[i], true);
  }
}
