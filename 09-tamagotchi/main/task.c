#include "task.h"

static Task tasks[MAX_TASKS] = {0};

int find_id(const char *id) {
  if (id == NULL)
    return -1;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id != NULL && strcmp(id, tasks[i].id) == 0)
      return i;
  }

  return -1;
}

int find_finished() {
  for (int i = 0; i < MAX_TASKS; i++)
    if (tasks[i].status == FINISHED)
      return i;
  return -1;
};

int task_start(const char *id, TaskHandler handler, void *args) {
  if (id == NULL || handler == NULL)
    return 1;
  if (find_id(id) != -1)
    return 1;
  int i = find_finished();
  if (i == -1)
    return 1;
  tasks[i] = (Task){.status = RUNNING, .id = id, .args = args, .handle = NULL};
  if (xTaskCreate(handler, id, TASK_STACK_SIZE, args, TASK_PRIORITY,
                  &tasks[i].handle) != pdPASS) {
    tasks[i] = (Task){0};
    return 1;
  }
  return 0;
}

int task_stop(const char *id) {
  int i = find_id(id);
  if (i == -1)
    return 1;
  vTaskDelete(tasks[i].handle);
  if (tasks[i].args != NULL)
    free(tasks[i].args);
  tasks[i].handle = NULL;
  tasks[i].id = NULL;
  tasks[i].status = FINISHED;
  return 0;
}
