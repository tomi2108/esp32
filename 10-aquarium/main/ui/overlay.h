#pragma once

#include <stdbool.h>
#include <stddef.h>

#define OVERLAY_STACK_SIZE 8

typedef struct Overlay Overlay;

struct Overlay {
  void (*update)(Overlay *self);
  void (*render)(Overlay *self);
  void *data;
};

bool overlay_show(Overlay *overlay);
bool overlay_remove(Overlay *overlay);

void overlay_update(void);
void overlay_render(void);
