#include "button.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"

Button button_init(gpio_num_t gpio) {
  gpio_set_direction(gpio, GPIO_MODE_INPUT);
  gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
  return (Button){.gpio = gpio};
};

int button_is_pressed(Button button) {
  return gpio_get_level(button.gpio) == 1;
}
