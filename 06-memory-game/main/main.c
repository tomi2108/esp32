#include "button.h"
#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "melody.h"
#include "oled.h"
#include "soc/gpio_num.h"
#include <stdint.h>

#define INPUTS 3

#define LED_DELAY 600

typedef struct Input {
  gpio_num_t led_pin;
  gpio_num_t button_pin;
  int frequency;
  Button button;
} Input;

Input inputs[INPUTS] = {
    (Input){.led_pin = GPIO_NUM_16,
            .button_pin = GPIO_NUM_32,
            .frequency = NOTE_A4},
    (Input){.led_pin = GPIO_NUM_19,
            .button_pin = GPIO_NUM_33,
            .frequency = NOTE_C4},
    (Input){.led_pin = GPIO_NUM_17,
            .button_pin = GPIO_NUM_25,
            .frequency = NOTE_B4},
};

PassiveBuzzer buzzer;
OLED oled = {0};
Button start_button;
gpio_num_t start_button_pin = GPIO_NUM_23;
gpio_num_t buzzer_pin = GPIO_NUM_27;

void init() {
  oled_init(&oled, 128, 64, GPIO_NUM_21, GPIO_NUM_22);
  start_button = button_init(start_button_pin);
  buzzer = passive_buzzer_init(buzzer_pin);
  for (int i = 0; i < INPUTS; i++)
    gpio_set_direction(inputs[i].led_pin, GPIO_MODE_OUTPUT);
  for (int i = 0; i < INPUTS; i++)
    inputs[i].button = button_init(inputs[i].button_pin);
}

int next_sequence_step() { return rand() % INPUTS; }

void led_on(gpio_num_t pin) { gpio_set_level(pin, 1); }
void led_off(void) {
  for (int i = 0; i < INPUTS; i++)
    gpio_set_level(inputs[i].led_pin, 0);
}

void show_sequence(int level, const uint8_t sequence[]) {
  led_off();
  for (int i = 0; i < level; i++) {
    uint8_t current_input = sequence[i];
    Input input = inputs[current_input];
    led_on(input.led_pin);
    buzzer_set_frequency(&buzzer, input.frequency);
    buzzer_tone(buzzer);
    vTaskDelay(pdMS_TO_TICKS(LED_DELAY));
    led_off();
    buzzer_stop();
    vTaskDelay(pdMS_TO_TICKS(LED_DELAY / 2));
  }
}

void pressed_correct_input(Input input) {
  buzzer_set_frequency(&buzzer, input.frequency);
  buzzer_tone(buzzer);
  led_on(input.led_pin);
  vTaskDelay(pdMS_TO_TICKS(LED_DELAY));
  led_off();
  buzzer_stop();
}

void show_score(int score) {
  oled_clear(&oled);
  oled_write_text(&oled, 0, 0, "Current Score: %d", score);
  oled_update(&oled);
}

int wait_for_input(int correct_index) {
  while (1) {
    for (int j = 0; j < INPUTS; j++) {
      Input input = inputs[j];
      if (!button_is_pressed(input.button))
        continue;
      wait_for_release(input.button);

      if (j == correct_index) {
        pressed_correct_input(input);
        return 0;
      }

      return 1;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
};

void game() {
  int level = 1;
  uint8_t sequence[100] = {};
  show_score(0);

  while (1) {
    sequence[level - 1] = next_sequence_step();
    show_sequence(level, sequence);
    for (int i = 0; i < level; i++) {
      int wrong = wait_for_input(sequence[i]);
      if (wrong)
        return;
    }
    show_score(level);
    level++;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  init();
  oled_write_text(&oled, 0, 0, "Press button to start");
  oled_update(&oled);
  while (1) {
    if (button_is_pressed(start_button)) {
      wait_for_release(start_button);
      game();
    }
    // TODO: Show lost/restart
  }
}
