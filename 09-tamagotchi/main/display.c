#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdlib.h>

static SemaphoreHandle_t oled_mutex;

void oled_lock(void) {
  if (oled_mutex == NULL)
    return;
  xSemaphoreTake(oled_mutex, portMAX_DELAY);
}

void oled_unlock(void) {
  if (oled_mutex == NULL)
    return;
  xSemaphoreGive(oled_mutex);
}

void display_init() {
  oled_mutex = xSemaphoreCreateMutex();
  if (oled_mutex == NULL)
    abort();
}
