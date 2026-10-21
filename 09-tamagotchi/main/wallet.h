#pragma once
#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "icons.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"
#include <inttypes.h>

#define MAX_SHOP_SIZE 50

void wallet_refresh_async(void);
void wallet_display(int x, int y);
