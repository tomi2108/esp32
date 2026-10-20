#pragma once
#include "./env.h"
#include "buzzer.h"
#include "wallet.h"
#include "esp_log.h"
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

typedef struct ShopItem {
  int32_t id;
  const char *name;
  const char *type;
  const char *slot;
  const char *rarity;
  int32_t price;
  int32_t stock;
} ShopItem;

void shop_refresh_async(void);
void shop_display(void);
