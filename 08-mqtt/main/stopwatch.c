#include "buzzer.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "rotary_encoder.h"
#include <stdint.h>

extern PassiveBuzzer buzzer;
extern int wholenote;
extern OLED oled;
extern RotaryEncoder rotary_encoder;

bool started = false;
int64_t start_time = 0;

void stopwatch_on_start() {
  started = false;
  start_time = 0;
}

void stopwatch_update(void) {
  int64_t elapsed_before_pause = 0;

  while (1) {
    if (rotary_encoder_is_pressed(&rotary_encoder)) {
      rotary_encoder_wait_for_release(&rotary_encoder);

      if (started) {
        elapsed_before_pause += esp_timer_get_time() - start_time;
        started = false;
      } else {
        start_time = esp_timer_get_time();
        started = true;
      }
    }

    int64_t elapsed = elapsed_before_pause;

    if (started)
      elapsed += esp_timer_get_time() - start_time;

    int64_t ms = elapsed / 1000;

    oled_clear(&oled);
    oled_write_text(&oled, 0, 0, false, "%lld ms", (long long)ms);
    oled_update(&oled);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
