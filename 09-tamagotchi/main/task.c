#include "task.h"

static Task tasks[MAX_TASKS] = {0};
static SemaphoreHandle_t tasks_mutex = NULL;

static void task_wrapper(void *args) {
  Task *task = args;
  TaskHandler handler = task->handler;
  void *handler_args = task->args;
  handler(handler_args);
  if (handler_args != NULL)
    free(handler_args);
  xSemaphoreTake(tasks_mutex, portMAX_DELAY);
  task->status = FINISHED;
  task->args = NULL;
  task->handle = NULL;
  xSemaphoreGive(tasks_mutex);
  vTaskDelete(NULL);
}

void task_manager_init(void) {
  tasks_mutex = xSemaphoreCreateMutex();
  if (tasks_mutex == NULL)
    abort();
}

int find_id(const char *id) {
  if (id == NULL || tasks_mutex == NULL)
    return -1;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int result = -1;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id != NULL && strcmp(id, tasks[i].id) == 0) {
      result = i;
      break;
    }
  }

  xSemaphoreGive(tasks_mutex);

  return result;
}

int find_finished(void) {
  if (tasks_mutex == NULL)
    return -1;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int result = -1;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id == NULL) {
      result = i;
      break;
    }
  }

  xSemaphoreGive(tasks_mutex);

  return result;
}

int task_start(const char *id, TaskHandler handler, void *args) {
  if (id == NULL || handler == NULL || tasks_mutex == NULL)
    return 1;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id != NULL && strcmp(id, tasks[i].id) == 0) {
      xSemaphoreGive(tasks_mutex);
      return 1;
    }
  }

  int i = -1;
  for (int j = 0; j < MAX_TASKS; j++) {
    if (tasks[j].id == NULL) {
      i = j;
      break;
    }
  }

  if (i == -1) {
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  tasks[i] = (Task){
      .status = RUNNING,
      .id = id,
      .handler = handler,
      .args = args,
      .handle = NULL,
  };

  if (xTaskCreate(task_wrapper, id, TASK_STACK_SIZE, &tasks[i], TASK_PRIORITY,
                  &tasks[i].handle) != pdPASS) {
    tasks[i] = (Task){0};
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  xSemaphoreGive(tasks_mutex);

  return 0;
}

int task_stop(const char *id) {
  if (id == NULL || tasks_mutex == NULL)
    return 1;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int i = -1;

  for (int j = 0; j < MAX_TASKS; j++) {
    if (tasks[j].id != NULL && strcmp(id, tasks[j].id) == 0) {
      i = j;
      break;
    }
  }

  if (i == -1) {
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  TaskHandle_t handle = tasks[i].handle;
  void *args = tasks[i].args;

  tasks[i] = (Task){0};

  xSemaphoreGive(tasks_mutex);

  if (args != NULL)
    free(args);

  if (handle != NULL)
    vTaskDelete(handle);

  return 0;
}
