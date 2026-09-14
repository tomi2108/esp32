#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"

#define BUTTON_GPIO GPIO_NUM_26
#define LED_GPIO GPIO_NUM_25

void app_main(void) {
  gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

  while (1) {
    if (gpio_get_level(BUTTON_GPIO) == 1) {
      gpio_set_level(LED_GPIO, 1);
      vTaskDelay(pdMS_TO_TICKS(5000));
    } else {
      gpio_set_level(LED_GPIO, 0);
    }
  }
}
