#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "inbox.c"
#include "json.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"
#include "wifi.h"

PassiveBuzzer buzzer;
int wholenote = (60000 * 4) / 140;
OLED oled;
RotaryEncoder rotary_encoder;

TaskHandle_t spinner_handle = NULL;

static void spinner_task(void *arg) {
  while (1) {
    oled_draw_spinner(&oled, 64, 32, 10);
    oled_update(&oled);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void init() {
  oled_init(&oled, 128, 64, SDA_PIN, SCL_PIN);
  oled.font = font_eight_by_eight();
  buzzer = passive_buzzer_init(BUZZER_PIN);
  rotary_encoder_init(&rotary_encoder, A_PIN, B_PIN, BUTTON_PIN);

  buzzer_play_melody(buzzer, wholenote, sound_boot());

  xTaskCreate(spinner_task, "spinner", 2048, NULL, 1, &spinner_handle);
  wifi_init(WIFI_SSID, WIFI_PASSWORD);

  vTaskDelete(spinner_handle);
  spinner_handle = NULL;
  vTaskDelay(pdMS_TO_TICKS(20));

  oled_clear(&oled);
  oled_update(&oled);

  buzzer_play_melody(buzzer, wholenote, sound_success());
}

void app_main(void) {
  init();

  // HttpClient *client = client_create(SERVER_URL);
  // Response *response = client_send_get(client, "/inbox/tomi");
  // t_json_value *body = json_parse_buffer(response->body);
  // const t_json_value *first_message = json_array_get(body, 0);
  // const char *sender_id =
  //     json_value_string(json_object_get(first_message, "sender_id"));
  // ESP_LOGI("JSON", "%s", sender_id);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  };
};
