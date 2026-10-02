#include "button.h"
#include "buzzer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lcd.h"
#include "melody.h"
#include "menu.h"
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

Option options[5] = {
    {.key = "415hz", .value = 415},
    {.key = "440hz", .value = 440},
    {.key = "466hz", .value = 466},
    {.key = "523hz", .value = 523},
};

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
      lcd_display_menu(lcd, menu);
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

    if (button_is_pressed(ok_b))
      menu_select(&menu);
  }
}
