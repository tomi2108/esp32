#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"
#include <sys/types.h>

typedef enum Mode {
  COMMON_CATHODE,
  COMMON_ANODE,
} Mode;

typedef struct LED {
  // a,b,c,d,e,f,g,dp
  gpio_num_t data_pins[8];
  // least significant to most significant
  gpio_num_t *common_pins;
  u_int8_t digits;
  Mode mode;
} LED;

LED led_init(gpio_num_t data_pins[8], gpio_num_t *common_pins,
             u_int8_t digits, Mode mode);
void led_display(LED led, u_int32_t n);
