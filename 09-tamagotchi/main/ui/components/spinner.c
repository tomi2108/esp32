#include "spinner.h"

extern OLED oled;

typedef struct {
  int x;
  int y;
} Args;

void spinner_render(Overlay *self) {
  Args *args = self->data;
  oled_draw_spinner(&oled, args->x, args->y, 10);
}

Overlay spinner = {.render = spinner_render};

void spinner_start(int x, int y) {
  Args *args = malloc(sizeof(Args));
  args->x = x;
  args->y = y;
  spinner.data = args;
  overlay_show(&spinner);
}

void spinner_stop(void) {
  free(spinner.data);
  overlay_remove(&spinner);
}
