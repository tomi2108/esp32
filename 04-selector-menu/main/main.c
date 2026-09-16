#include "button.h"
#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lcd.h"
#include "melody.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#define BUTTON_OK_GPIO GPIO_NUM_32
#define BUTTON_UP_GPIO GPIO_NUM_25
#define BUTTON_DOWN_GPIO GPIO_NUM_26
#define BUZZER_GPIO GPIO_NUM_33
#define LEDS 4

#define LCD_SDA GPIO_NUM_21
#define LCD_SCL GPIO_NUM_22

typedef struct Option {
  char *key;
  int32_t value;
} Option;

typedef struct Menu {
  Option *options;
  int length;
  int selected;

  void *context;
  void (*on_next)(void *context);
  void (*on_prev)(void *context);
  void (*on_select)(void *context, int32_t value);
} Menu;

Option options[5] = {
    {.key = "415hz", .value = 415},
    {.key = "440hz", .value = 440},
    {.key = "466hz", .value = 466},
    {.key = "523hz", .value = 523},
};

void menu_next(Menu *menu) {
  menu->selected = (menu->selected + 1) % menu->length;
  menu->on_next(menu->context);
}

void menu_prev(Menu *menu) {
  menu->selected = (menu->selected + menu->length - 1) % menu->length;
  menu->on_prev(menu->context);
}

void menu_select(Menu *menu) {
  menu->on_select(menu->context, menu->options[menu->selected].value);
}

void menu_display(LCD lcd, Menu menu) {
  lcd_clear(lcd);
  char first_line[lcd.columns + 1];
  char *selected_option = menu.options[menu.selected].key;
  snprintf(first_line, sizeof(first_line), ">%s", selected_option);
  lcd_print_line(lcd, 0, first_line);

  for (int i = 1; i < lcd.rows; i++) {
    char next_line[lcd.columns + 1];
    char *next_option = menu.options[(menu.selected + 1) % menu.length].key;
    snprintf(next_line, sizeof(next_line), " %s", next_option);
    lcd_print_line(lcd, i, next_line);
  }
}

void on_next(void *context) {
  PassiveBuzzer *buzzer = (PassiveBuzzer *)context;
  buzzer_set_frequency(buzzer, 550);
  buzzer_tone_ms(*buzzer, 100);
}
void on_prev(void *context) {
  PassiveBuzzer *buzzer = (PassiveBuzzer *)context;
  buzzer_set_frequency(buzzer, 393);
  buzzer_tone_ms(*buzzer, 100);
}

void on_select(void *context, int32_t value) {
  PassiveBuzzer *buzzer = (PassiveBuzzer *)context;
  buzzer_set_frequency(buzzer, value);
  buzzer_tone(*buzzer);
}

void app_main(void) {
  PassiveBuzzer buzzer = passive_buzzer_init(BUZZER_GPIO);

  Menu menu = {.options = options,
               .length = 4,
               .selected = 0,
               .context = &buzzer,
               .on_next = &on_next,
               .on_prev = &on_prev,
               .on_select = &on_select};

  LCD lcd = lcd_init(2, 16, LCD_SDA, LCD_SCL);
  Button ok_b = button_init(BUTTON_OK_GPIO);
  Button up_b = button_init(BUTTON_UP_GPIO);
  Button d_b = button_init(BUTTON_DOWN_GPIO);
  int menu_displayed = 0;

  while (1) {
    if (!menu_displayed) {
      menu_display(lcd, menu);
      menu_displayed = 1;
    }

    if (button_is_pressed(d_b)) {
      menu_next(&menu);
      menu_displayed = 0;
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (button_is_pressed(up_b)) {
      menu_prev(&menu);
      menu_displayed = 0;
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (button_is_pressed(ok_b)) {
      menu_select(&menu);
    }
  }
}
