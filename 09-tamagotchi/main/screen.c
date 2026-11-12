#include "screen.h"

static Screen screen_stack[SCREEN_STACK_SIZE];
static int screen_stack_length = 0;

bool screen_push(Screen screen) {
  if (screen_stack_length >= SCREEN_STACK_SIZE)
    return false;
  screen_stack[screen_stack_length++] = screen;
  return true;
}

bool screen_pop(void) {
  if (screen_stack_length <= 1)
    return false;
  screen_stack_length--;
  return true;
}

bool screen_replace(Screen screen) {
  if (screen_stack_length == 0)
    return screen_push(screen);
  screen_stack[screen_stack_length - 1] = screen;
  return true;
}

Screen *screen_current(void) {
  if (screen_stack_length == 0)
    return NULL;
  return &screen_stack[screen_stack_length - 1];
}
