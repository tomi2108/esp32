#include "driver/dac_oneshot.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

void analog_init(void);
int analog_read(gpio_num_t gpio);
void analog_write(gpio_num_t gpio, uint8_t value);
