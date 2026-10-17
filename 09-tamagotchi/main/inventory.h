#pragma once

#include "./env.h"
#include "actions.h"
#include "buzzer.h"
#include "client.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json.h"
#include "menu.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_INVENTORY_SIZE 150
#define INVENTORY_LABEL_SIZE 64

void inventory_refresh_async(void);
void inventory_display(void);
