#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"
#include <stdint.h>
#include <sys/types.h>

typedef enum {
  ROTARY_NONE = 0,
  ROTARY_CLOCKWISE = 1,
  ROTARY_COUNTERCLOCKWISE = -1,
} RotaryDirection;

typedef struct RotaryEncoder {
  gpio_num_t a;
  gpio_num_t b;
  gpio_num_t button;
  int32_t position;
  int8_t last_state;
  int step;
  volatile RotaryDirection direction;
} RotaryEncoder;

void rotary_encoder_init(RotaryEncoder *encoder, gpio_num_t a, gpio_num_t b,
                         gpio_num_t button);
int32_t rotary_encoder_position(RotaryEncoder *encoder);
RotaryDirection rotary_encoder_direction(RotaryEncoder *encoder);
bool rotary_encoder_is_pressed(RotaryEncoder *encoder);
