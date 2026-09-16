#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"

typedef struct Button {
  gpio_num_t gpio;
} Button;

Button button_init(gpio_num_t gpio);
int button_is_pressed(Button button);
