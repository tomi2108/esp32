#include "seven_segment.h"

u_int8_t digits[10] = {
    0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
    0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110,
};

SevenSegment ss_int(gpio_num_t data_pins[8], gpio_num_t *common_pins,
                    uint8_t digits, Mode mode) {
  SevenSegment led = {
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

void ss_display(SevenSegment led, u_int32_t n) {
  static int j = 0;

  uint32_t divisor = 1;
  for (int i = 0; i < j; i++)
    divisor *= 10;

  uint8_t digit = (n / divisor) % 10;
  int on = led.mode == COMMON_CATHODE;

  for (int k = 0; k < led.digits; k++)
    gpio_set_level(led.common_pins[k], on);

  uint8_t mask = digits[digit];

  for (int i = 0; i < 8; i++) {
    bool segment_on = mask & (1 << (7 - i));
    gpio_set_level(led.data_pins[i], segment_on == on);
  }

  gpio_set_level(led.common_pins[j], !on);

  j = (j + 1) % led.digits;
}
