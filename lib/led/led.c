#include "led.h"

LED led_init(gpio_num_t data_pins[8], gpio_num_t *common_pins, uint8_t digits,
             Mode mode) {
  LED led = {
      .mode = mode,
      .digits = digits,
      .common_pins = common_pins,
  };

  for (int i = 0; i < 8; i++) {
    led.data_pins[i] = data_pins[i];
    gpio_set_direction(led.data_pins[i], GPIO_MODE_OUTPUT);
  }

  if (common_pins != NULL) {
    for (int i = 0; i < digits; i++)
      gpio_set_direction(common_pins[i], GPIO_MODE_OUTPUT);
  }

  return led;
}

u_int8_t digits[10] = {
    0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
    0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110,
};

void led_display(LED led, u_int32_t n) {
  int on = led.mode == COMMON_CATHODE;
  uint32_t divisor = 1;

  for (int j = 0; j < led.digits; j++) {
    uint8_t digit = (int)(n / divisor) % 10;

    if (led.common_pins != NULL) {
      for (int k = 0; k < led.digits; k++)
        gpio_set_level(led.common_pins[k], on);
    }

    for (int i = 0; i < 8; i++) {
      u_int8_t mask = digits[digit];
      bool segment_on = mask & (1 << (7 - i));
      gpio_set_level(led.data_pins[i], segment_on == on);
    }

    if (led.common_pins != NULL) {
      gpio_set_level(led.common_pins[j], !on);
    }

    vTaskDelay(pdMS_TO_TICKS(2));
    divisor *= 10;
  }
}
