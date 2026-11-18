#pragma once

#include "buzzer.h"
#include "client.h"
#include "common/task.h"
#include "env.h"
#include "json.h"
#include "mqttt.h"
#include "response.h"

#define INBOX_SIZE 100

typedef struct Message {
  const char *id;
  const char *content;
  bool is_read;
} Message;

int inbox_get_size(void);
int inbox_get_unread(void);
void inbox_read(int index);
void inbox_init(void);
Message inbox_get_message(int index);
Message *inbox_get();
