#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "soc/gpio_num.h"

void buzzer_init(gpio_num_t buzzer_gpio);
void buzzer_tone(uint32_t frequency);
void buzzer_tone_ms(uint32_t frequency, uint32_t ms);
void buzzer_stop(void);
