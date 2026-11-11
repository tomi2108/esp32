#include "spinner.h"
#include "freertos/idf_additions.h"
#include "task.h"

typedef struct {
  int x;
  int y;
  OLED *oled;
} Args;

static void spinner_task(void *arg) {
  Args *args = (Args *)arg;
  for (;;) {
    oled_lock();
    oled_clear(args->oled);
    oled_draw_spinner(args->oled, args->x, args->y, 10);
    oled_update(args->oled);
    oled_unlock();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void spinner_start(OLED *oled, int x, int y) {
  Args *args = malloc(sizeof(Args));
  args->oled = oled;
  args->x = x;
  args->y = y;
  task_start("spinner", spinner_task, args);
}

void spinner_stop(void) { task_stop("spinner"); }
