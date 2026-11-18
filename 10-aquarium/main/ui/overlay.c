#include "overlay.h"

static Overlay *overlay_stack[OVERLAY_STACK_SIZE];
static int overlay_stack_length = 0;

bool overlay_show(Overlay *overlay) {
  if (overlay == NULL || overlay_stack_length >= OVERLAY_STACK_SIZE)
    return false;

  overlay_stack[overlay_stack_length++] = overlay;
  return true;
}

bool overlay_remove(Overlay *overlay) {
  if (overlay == NULL)
    return false;

  for (int i = overlay_stack_length - 1; i >= 0; i--) {
    if (overlay_stack[i] == overlay) {
      for (int j = i; j < overlay_stack_length - 1; j++)
        overlay_stack[j] = overlay_stack[j + 1];

      overlay_stack[--overlay_stack_length] = NULL;
      return true;
    }
  }

  return false;
}

Overlay *overlay_top(void) {
  if (overlay_stack_length == 0)
    return NULL;

  return overlay_stack[overlay_stack_length - 1];
}

void overlay_update(void) {
  Overlay *top = overlay_top();

  if (top && top->update)
    top->update(top);
}

void overlay_render(void) {
  for (int i = 0; i < overlay_stack_length; i++) {
    if (overlay_stack[i]->render)
      overlay_stack[i]->render(overlay_stack[i]);
  }
}
