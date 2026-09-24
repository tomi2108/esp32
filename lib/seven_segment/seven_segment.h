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

typedef struct SevenSegment {
  // a,b,c,d,e,f,g,dp
  gpio_num_t data_pins[8];
  // least significant to most significant
  gpio_num_t *common_pins;
  u_int8_t digits;
  Mode mode;
} SevenSegment;

SevenSegment ss_init(gpio_num_t data_pins[8], gpio_num_t *common_pins,
                     u_int8_t digits, Mode mode);
void ss_display(SevenSegment led, u_int32_t n);
