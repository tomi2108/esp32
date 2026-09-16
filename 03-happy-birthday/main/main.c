#include "button.h"
#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lcd.h"
#include "melody.h"
#include <stdint.h>

#define BUTTON_GPIO GPIO_NUM_4
#define BUZZER_GPIO GPIO_NUM_16
#define LEDS 4

#define LCD_SDA GPIO_NUM_21
#define LCD_SCL GPIO_NUM_22

int ledPins[LEDS] = {
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_25,
    GPIO_NUM_26,
};

void leds_init() {
  for (int i = 0; i < LEDS; i++)
    gpio_set_direction(ledPins[i], GPIO_MODE_OUTPUT);
}

void leds_set(int index) {
  for (int i = 0; i < LEDS; i++)
    gpio_set_level(ledPins[i], i == index);
}

void leds_stop() {
  for (int i = 0; i < LEDS; i++)
    gpio_set_level(ledPins[i], 0);
}

int default_message;

void lcd_default_message(LCD lcd) {
  if (default_message)
    return;
  default_message = 1;
  lcd_clear(lcd);
  lcd_print_line(lcd, 0, "No tocar");
  lcd_print_line(lcd, 1, "Excepto Rochi :)");
}

void lcd_on_message(LCD lcd) {
  if (!default_message)
    return;
  default_message = 0;
  lcd_clear(lcd);
  lcd_print_line(lcd, 0, "Feliz cumplee!!!");
  lcd_print_line(lcd, 1, "Te amo muchoo <3");
}

int wholenote = (60000 * 4) / 140;
void app_main(void) {
  leds_init();

  PassiveBuzzer buzzer = passive_buzzer_init(BUZZER_GPIO);

  LCD lcd = lcd_init(LCD_SDA, LCD_SCL);
  lcd_default_message(lcd);

  Button button = button_init(BUTTON_GPIO);

  Note *melody = NULL;
  int notes = melody_happy_birthday(&melody);

  while (1) {
    if (button_is_pressed(button)) {
      lcd_on_message(lcd);
      for (int i = 0; i < notes; i++) {
        Note note = melody[i];
        uint32_t duration = duration_to_ms(note.duration, wholenote);
        leds_set(i % LEDS);
        buzzer_set_frequency(&buzzer, note.frequency);
        buzzer_tone_ms(buzzer, duration * 0.9);
        vTaskDelay(pdMS_TO_TICKS(duration * 0.1));
      }
    } else {
      lcd_default_message(lcd);
      buzzer_stop();
      leds_stop();
    }
  }
}
