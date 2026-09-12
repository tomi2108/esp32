#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_25
#define LED_GPIO2 GPIO_NUM_26

void app_main(void)
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 1);
        gpio_set_level(LED_GPIO2, 0);
        vTaskDelay(pdMS_TO_TICKS(200));

        gpio_set_level(LED_GPIO, 0);
        gpio_set_level(LED_GPIO2, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
