#include "buzzer.h"

void buzzer_init(gpio_num_t buzzer_gpio) {
  gpio_set_direction(buzzer_gpio, GPIO_MODE_OUTPUT);
  ledc_timer_config_t timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .freq_hz = 440,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ledc_channel_config_t channel = {
      .gpio_num = buzzer_gpio,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0,
  };
  ledc_timer_config(&timer);
  ledc_channel_config(&channel);
}

void buzzer_tone(uint32_t frequency) {
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_stop(void) {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_tone_ms(uint32_t frequency, uint32_t ms) {
  buzzer_tone(frequency);
  vTaskDelay(pdMS_TO_TICKS(ms));
  buzzer_stop();
}
