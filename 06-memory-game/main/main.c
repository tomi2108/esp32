#include "button.h"
#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lcd.h"
#include "melody.h"
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
Button start_button;
gpio_num_t start_button_pin = GPIO_NUM_23;
gpio_num_t buzzer_pin = GPIO_NUM_27;

void init() {
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

  while (1) {
    // TODO: Show current level
    sequence[level - 1] = next_sequence_step();
    show_sequence(level, sequence);
    for (int i = 0; i < level; i++) {
      int wrong = wait_for_input(sequence[i]);
      if (wrong)
        return;
    }
    level++;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void) {
  init();
  // TODO: Show start game menu
  while (1) {
    if (button_is_pressed(start_button)) {
      wait_for_release(start_button);

      game();
    }
    // TODO: Show lost/restart
  }
}
