#pragma once

#include <stdbool.h>
#include <stddef.h>

#define SCREEN_STACK_SIZE 20

typedef struct Screen Screen;

struct Screen {
  void (*on_enter)(Screen *self);
  void (*on_exit)(Screen *self);
  void (*update)(Screen *self);
  void (*render)(Screen *self);
  void *context;
};

bool screen_push(Screen screen);
bool screen_pop(void);
bool screen_replace(Screen screen);
Screen *screen_current(void);
