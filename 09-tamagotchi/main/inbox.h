#pragma once

#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "esp_log.h"
#include "icons.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define INBOX_SIZE 100

typedef struct Message {
  char id[16];
  char content[512];
  bool is_read;
} Message;

void inbox_display(void);
void inbox_init(void);
