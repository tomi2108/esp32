#include "analog.h"

static adc_oneshot_unit_handle_t adc;
static dac_oneshot_handle_t dac25;
static dac_oneshot_handle_t dac26;

void analog_init(void) {
  adc_oneshot_unit_init_cfg_t adc_config = {
      .unit_id = ADC_UNIT_1,
  };
  dac_oneshot_config_t dac25_config = {
      .chan_id = DAC_CHAN_0,
  };
  dac_oneshot_config_t dac26_config = {
      .chan_id = DAC_CHAN_1,
  };

  adc_oneshot_new_unit(&adc_config, &adc);
  dac_oneshot_new_channel(&dac25_config, &dac25);
  dac_oneshot_new_channel(&dac26_config, &dac26);
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
    return -1;
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

void analog_write(gpio_num_t gpio, uint8_t value) {
  switch (gpio) {
  case GPIO_NUM_25:
    dac_oneshot_output_voltage(dac25, value);
    break;

  case GPIO_NUM_26:
    dac_oneshot_output_voltage(dac26, value);
    break;

  default:
    break;
  }
}
