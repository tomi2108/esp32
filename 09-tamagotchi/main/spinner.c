#include "spinner.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include <stdlib.h>

#define SPINNER_TASK_STACK_SIZE 4096
#define SPINNER_TASK_PRIORITY 2
#define SPINNER_FRAME_DELAY_MS 100

static TaskHandle_t spinner_task_handle;
static OLED *spinner_oled;

typedef struct {
  int x;
  int y;
} Args;

static void spinner_task(void *arg) {
  Args *args = (Args *)arg;

  for (;;) {
    oled_lock();
    oled_clear(spinner_oled);
    oled_draw_spinner(spinner_oled, args->x, args->y, 10);
    oled_update(spinner_oled);
    oled_unlock();

    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SPINNER_FRAME_DELAY_MS)))
      break;
  }
  free(args);
  spinner_task_handle = NULL;
  vTaskDelete(NULL);
}

void spinner_start(OLED *oled, int x, int y) {
  if (spinner_task_handle != NULL)
    return;
  spinner_oled = oled;
  Args *args = malloc(sizeof(Args));
  args->x = x;
  args->y = y;
  xTaskCreate(spinner_task, "spinner", SPINNER_TASK_STACK_SIZE, args,
              SPINNER_TASK_PRIORITY, &spinner_task_handle);
}

void spinner_stop(void) {
  TaskHandle_t task = spinner_task_handle;
  if (task == NULL)
    return;
  xTaskNotifyGive(task);
  while (spinner_task_handle == task)
    vTaskDelay(1);
}
