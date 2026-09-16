#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"

typedef struct PassiveBuzzer {
  gpio_num_t gpio;
  uint32_t frequency;
} PassiveBuzzer;

PassiveBuzzer passive_buzzer_init(gpio_num_t gpio);
void buzzer_set_frequency(PassiveBuzzer *buzzer, uint32_t frequency);
void buzzer_tone(PassiveBuzzer buzzer);
void buzzer_tone_ms(PassiveBuzzer buzzer, uint32_t ms);
void buzzer_stop();
