#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

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
  TickType_t last_click_tick;
  TickType_t button_down_tick;
  bool was_pressed;
  bool waiting_for_double;
} RotaryEncoder;

typedef enum {
  ROTARY_CLICK_NONE,
  ROTARY_CLICK_SINGLE,
  ROTARY_CLICK_DOUBLE
} RotaryClick;

void rotary_encoder_init(RotaryEncoder *encoder, gpio_num_t a, gpio_num_t b,
                         gpio_num_t button);
int32_t rotary_encoder_position(RotaryEncoder *encoder);
RotaryDirection rotary_encoder_direction(RotaryEncoder *encoder);
void rotary_encoder_wait_for_release(RotaryEncoder *encoder);

RotaryClick rotary_encoder_get_click(RotaryEncoder *encoder);
#endif
