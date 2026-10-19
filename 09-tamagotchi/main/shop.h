#pragma once
#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define MAX_SHOP_SIZE 50

typedef struct ShopItem {
  int id;
  const char *name;
  const char *type;
  const char *slot;
  const char *rarity;
  int price;
  int stock;
} ShopItem;

void shop_refresh_async(void);
void shop_display(void);
