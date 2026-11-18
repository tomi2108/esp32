#include "buzzer.h"
#include "client.h"
#include "common/task.h"
#include "env.h"
#include "mqttt.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/aquarium.h"
#include "ui/screen.h"
#include "ui/views/aquarium.h"
#include "wifi.h"
#include <stdint.h>

#define FPS 40
#define FRAME_INTERVAL pdMS_TO_TICKS(1000 / FPS)

extern const uint8_t certificate[] asm("_binary_mosquitto_ca_crt_start");

PassiveBuzzer buzzer;
int wholenote = (60000 * 4) / 140;
OLED oled;
RotaryEncoder rotary_encoder;
MqttClient mqtt_client;
HttpClient *http_client;

void modules_init() { task_manager_init(); }

static void show_progress(uint8_t progress, const char *message) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 12, false, "%s", message);
  oled_draw_progress_bar(&oled, 14, 30, 100, 8, progress, false);
  oled_write_text(&oled, 52, 46, false, "%d%%", progress);
  oled_update(&oled);
}

static void init() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN, OLED_CONTROLLER_SH1106);
  oled.font = font_default();
  show_progress(0, "Iniciando");

  buzzer = passive_buzzer_init(BUZZER_PIN);
  rotary_encoder_init(&rotary_encoder, A_PIN, B_PIN, BUTTON_PIN);
  buzzer_play_melody(buzzer, wholenote, sound_boot());

  show_progress(70, "");
  modules_init();
  show_progress(100, "");
  buzzer_play_melody(buzzer, wholenote, sound_success());
}

void app_main(void) {
  init();
  aquarium_view();
  aquarium_add(FISH_GUPPY);

  TickType_t last_wake_time = xTaskGetTickCount();

  while (1) {
    oled_clear(&oled);
    Screen *screen = screen_current();
    if (screen->update)
      screen->update(screen);
    overlay_update();
    if (screen->render)
      screen->render(screen);
    overlay_render();
    oled_update(&oled);
    vTaskDelayUntil(&last_wake_time, FRAME_INTERVAL);
  }
}
