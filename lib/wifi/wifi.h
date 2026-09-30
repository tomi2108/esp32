#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "soc/gpio_num.h"
#include <string.h>
#include <sys/types.h>

void wifi_init(const char *ssid, const char *password);
