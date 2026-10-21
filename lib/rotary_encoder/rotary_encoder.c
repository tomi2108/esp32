#include "rotary_encoder.h"
#define DEBOUNCE_MS 30
#define DOUBLE_CLICK_MS 500

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

RotaryClick rotary_encoder_get_click(RotaryEncoder *encoder) {
    TickType_t now = xTaskGetTickCount();
    bool pressed = rotary_encoder_is_pressed(encoder);

    if (pressed && !encoder->was_pressed) {
        encoder->button_down_tick = now;
    }

    if (!pressed && encoder->was_pressed) {
        TickType_t held_ms = pdTICKS_TO_MS(
            now - encoder->button_down_tick
        );

        if (held_ms >= DEBOUNCE_MS) {
            if (encoder->waiting_for_double &&
                pdTICKS_TO_MS(now - encoder->last_click_tick)
                    <= DOUBLE_CLICK_MS) {
                encoder->waiting_for_double = false;
                encoder->was_pressed = pressed;
                return ROTARY_CLICK_DOUBLE;
            }

            encoder->last_click_tick = now;
            encoder->waiting_for_double = true;
        }
    }

    encoder->was_pressed = pressed;

    if (encoder->waiting_for_double &&
        pdTICKS_TO_MS(now - encoder->last_click_tick)
            > DOUBLE_CLICK_MS) {
        encoder->waiting_for_double = false;
        return ROTARY_CLICK_SINGLE;
    }

    return ROTARY_CLICK_NONE;
}

void rotary_encoder_wait_for_release(RotaryEncoder *encoder) {
  while (rotary_encoder_is_pressed(encoder))
    vTaskDelay(pdMS_TO_TICKS(200));
}
