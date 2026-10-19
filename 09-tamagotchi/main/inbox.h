#pragma once

#include "./env.h"
#include "buzzer.h"
#include "client.h"
#include "esp_log.h"
#include "json.h"
#include "menu.h"
#include "mqttt.h"
#include "oled.h"
#include "response.h"
#include "rotary_encoder.h"

#define INBOX_SIZE 500

typedef struct Message {
  const char *id;
  const char *content;
  bool is_read;
} Message;

void inbox_display(void);
void inbox_init(void);
