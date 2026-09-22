#include "potentiometer.h"

Potentiometer potentiometer_init(gpio_num_t gpio) {
  Potentiometer p = {.gpio = gpio};
  analog_init();
  gpio_set_direction(gpio, GPIO_MODE_INPUT);
  return p;
};

float potentiometer_read(Potentiometer potentiometer) {
  float sum = 0;
  for (int i = 0; i < 16; i++)
    sum += analog_read(potentiometer.gpio);
  return sum / (16.0f * 4095.0f);
}
