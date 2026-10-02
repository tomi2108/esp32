#include "rotary_encoder.h"

int decode(uint8_t last, uint8_t current) {
  static const int8_t table[4][4] = {
      {0, -1, +1, 0},
      {+1, 0, 0, -1},
      {-1, 0, 0, +1},
      {0, +1, -1, 0},
  };
  return table[last][current];
}

static void IRAM_ATTR rotary_encoder_isr(void *arg) {
  RotaryEncoder *encoder = arg;

  uint8_t state =
      (gpio_get_level(encoder->a) << 1) | gpio_get_level(encoder->b);

  int direction = decode(encoder->last_state, state);

  encoder->step += direction;
  encoder->last_state = state;

  if (encoder->step >= 2) {
    encoder->position++;
    encoder->direction = ROTARY_CLOCKWISE;
    encoder->step = 0;
  } else if (encoder->step <= -2) {
    encoder->position--;
    encoder->direction = ROTARY_COUNTERCLOCKWISE;
    encoder->step = 0;
  }
}

void rotary_encoder_init(RotaryEncoder *encoder, gpio_num_t a, gpio_num_t b,
                         gpio_num_t button) {
  encoder->a = a;
  encoder->b = b;
  encoder->button = button;
  encoder->position = 0;
  encoder->step = 0;
  encoder->last_state = (gpio_get_level(a) << 1) | gpio_get_level(b);

  gpio_set_direction(a, GPIO_MODE_INPUT);
  gpio_set_direction(b, GPIO_MODE_INPUT);
  gpio_set_direction(button, GPIO_MODE_INPUT);

  gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);

  gpio_set_intr_type(a, GPIO_INTR_ANYEDGE);
  gpio_set_intr_type(b, GPIO_INTR_ANYEDGE);

  gpio_install_isr_service(0);

  gpio_isr_handler_add(a, rotary_encoder_isr, encoder);
  gpio_isr_handler_add(b, rotary_encoder_isr, encoder);
}

int32_t rotary_encoder_position(RotaryEncoder *encoder) {
  return encoder->position;
}

RotaryDirection rotary_encoder_direction(RotaryEncoder *encoder) {
  RotaryDirection direction = encoder->direction;
  encoder->direction = ROTARY_NONE;
  return direction;
}

bool rotary_encoder_is_pressed(RotaryEncoder *encoder) {
  return !gpio_get_level(encoder->button);
}
