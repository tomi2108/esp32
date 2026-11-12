#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include "ui/overlay.h"
#include <stdlib.h>

void spinner_start(int x, int y);
void spinner_stop(void);
