#pragma once

#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include "task.h"
#include <stdlib.h>

void spinner_start(OLED *oled, int x, int y);
void spinner_stop(void);
