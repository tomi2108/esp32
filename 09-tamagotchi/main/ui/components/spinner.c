#include "spinner.h"
#include "oled.h"

extern OLED oled;

typedef struct {
  int x;
  int y;
} Args;

void spinner_render(Overlay *self) {
  Args *args = self->data;
  int cx = (oled.width / 2);
  int cy = (oled.height / 2);
  int border = 1;
  int inner = 6;
  int outer = inner + border;
  oled_fill_square(&oled, cx - outer, cy - outer, outer * 2, true);
  oled_fill_square(&oled, cx - inner, cy - inner, inner * 2, false);
  oled_draw_spinner(&oled, args->x, args->y, 8);
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
