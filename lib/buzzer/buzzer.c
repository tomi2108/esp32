#include "buzzer.h"

PassiveBuzzer passive_buzzer_init(gpio_num_t gpio) {
  gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
  uint32_t initial_freq = 440;
  ledc_timer_config_t timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .freq_hz = 440,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ledc_channel_config_t channel = {
      .gpio_num = gpio,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0,
  };
  ledc_timer_config(&timer);
  ledc_channel_config(&channel);
  return (PassiveBuzzer){
      .frequency = initial_freq,
      .gpio = gpio,
  };
}

void buzzer_set_frequency(PassiveBuzzer *buzzer, uint32_t frequency) {
  buzzer->frequency = frequency;
};

void buzzer_tone(PassiveBuzzer buzzer) {
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, buzzer.frequency);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_stop(void) {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_tone_ms(PassiveBuzzer buzzer, uint32_t ms) {
  buzzer_tone(buzzer);
  vTaskDelay(pdMS_TO_TICKS(ms));
  buzzer_stop();
}
