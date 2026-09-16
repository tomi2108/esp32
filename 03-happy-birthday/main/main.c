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

static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t lcd_dev;

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

void lcd_default_message() {
  if (default_message)
    return;
  default_message = 1;
  lcd_clear(lcd_dev);
  lcd_set_cursor(lcd_dev, 0, 0);
  lcd_print(lcd_dev, "No tocar");
  lcd_set_cursor(lcd_dev, 1, 0);
  lcd_print(lcd_dev, "Excepto Rochi :)");
}

void lcd_on_message() {
  if (!default_message)
    return;
  default_message = 0;
  lcd_clear(lcd_dev);
  lcd_set_cursor(lcd_dev, 0, 0);
  lcd_print(lcd_dev, "Feliz cumplee!!!");
  lcd_set_cursor(lcd_dev, 1, 0);
  lcd_print(lcd_dev, "Te amo muchoo <3");
}

int wholenote = (60000 * 4) / 140;
void app_main(void) {
  leds_init();
  buzzer_init(BUZZER_GPIO);

  lcd_init(&i2c_bus, &lcd_dev, LCD_SDA, LCD_SCL);
  lcd_default_message();

  Button button = button_init(BUTTON_GPIO);
  Note *melody = NULL;
  int notes = melody_happy_birthday(&melody);

  while (1) {
    if (button_is_pressed(button)) {
      lcd_on_message();

      for (int i = 0; i < notes; i++) {
        Note note = melody[i];
        uint32_t noteDuration = duration_to_ms(note.duration, wholenote);
        leds_set(i % LEDS);
        buzzer_tone_ms(note.note, noteDuration * 0.9);
        vTaskDelay(pdMS_TO_TICKS(noteDuration * 0.1));
      }

    } else {
      lcd_default_message();
      buzzer_stop();
      leds_stop();
    }
  }
}
