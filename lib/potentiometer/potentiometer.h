#include "analog.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"

typedef struct Potentiometer {
  gpio_num_t gpio;
} Potentiometer;

Potentiometer potentiometer_init(gpio_num_t gpio);
float potentiometer_read(Potentiometer potentiometer);
