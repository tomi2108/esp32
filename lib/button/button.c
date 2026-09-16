#include "button.h"

Button button_init(gpio_num_t gpio) {
  gpio_set_direction(gpio, GPIO_MODE_INPUT);
  return (Button){.gpio = gpio};
};

int button_is_pressed(Button button) {
  return gpio_get_level(button.gpio) == 1;
}
