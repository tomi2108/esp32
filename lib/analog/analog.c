#include "analog.h"

static adc_oneshot_unit_handle_t adc;

void analog_init(void) {
  adc_oneshot_unit_init_cfg_t config = {
      .unit_id = ADC_UNIT_1,
  };

  adc_oneshot_new_unit(&config, &adc);
}

int analog_read(gpio_num_t gpio) {
  adc_channel_t channel;

  switch (gpio) {
  case GPIO_NUM_32:
    channel = ADC_CHANNEL_4;
    break;
  case GPIO_NUM_33:
    channel = ADC_CHANNEL_5;
    break;
  case GPIO_NUM_34:
    channel = ADC_CHANNEL_6;
    break;
  case GPIO_NUM_35:
    channel = ADC_CHANNEL_7;
    break;
  case GPIO_NUM_36:
    channel = ADC_CHANNEL_0;
    break;
  case GPIO_NUM_37:
    channel = ADC_CHANNEL_1;
    break;
  case GPIO_NUM_38:
    channel = ADC_CHANNEL_2;
    break;
  case GPIO_NUM_39:
    channel = ADC_CHANNEL_3;
    break;
  default:
    return -1.0f;
  }

  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  adc_oneshot_config_channel(adc, channel, &config);

  int value;
  adc_oneshot_read(adc, channel, &value);
  return value;
}
