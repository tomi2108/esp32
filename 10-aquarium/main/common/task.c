#include "task.h"

static Task tasks[MAX_TASKS] = {0};
static SemaphoreHandle_t tasks_mutex = NULL;
static EventGroupHandle_t completion_events = NULL;

static EventBits_t task_bit(int index) { return (EventBits_t)(1UL << index); }

static int find_id_locked(const char *id) {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id != NULL && strcmp(id, tasks[i].id) == 0)
      return i;
  }

  return -1;
}

static void task_wrapper(void *args) {
  Task *task = args;
  TaskHandler handler = task->handler;
  void *handler_args = task->args;

  handler(handler_args);

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int index = task - tasks;

  task->status = FINISHED;
  task->args = NULL;
  task->handle = NULL;

  xEventGroupSetBits(completion_events, task_bit(index));

  if (task->waiters == 0)
    *task = (Task){0};

  xSemaphoreGive(tasks_mutex);

  if (handler_args != NULL)
    free(handler_args);

  vTaskDelete(NULL);
}

void task_manager_init(void) {
  tasks_mutex = xSemaphoreCreateMutex();
  completion_events = xEventGroupCreate();

  if (tasks_mutex == NULL || completion_events == NULL)
    abort();
}

int task_start(const char *id, TaskHandler handler, void *args) {
  if (id == NULL || handler == NULL || tasks_mutex == NULL)
    return 1;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  if (find_id_locked(id) != -1) {
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  int index = -1;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].id == NULL) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  xEventGroupClearBits(completion_events, task_bit(index));

  tasks[index] = (Task){
      .status = RUNNING,
      .id = id,
      .handler = handler,
      .args = args,
      .handle = NULL,
      .waiters = 0,
  };

  if (xTaskCreate(task_wrapper, id, TASK_STACK_SIZE, &tasks[index],
                  TASK_PRIORITY, &tasks[index].handle) != pdPASS) {
    tasks[index] = (Task){0};
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

  int index = find_id_locked(id);

  if (index == -1) {
    xSemaphoreGive(tasks_mutex);
    return 1;
  }

  Task *task = &tasks[index];
  TaskHandle_t handle = task->handle;
  void *args = task->args;

  task->status = FINISHED;
  task->handle = NULL;
  task->args = NULL;

  xEventGroupSetBits(completion_events, task_bit(index));

  if (handle != NULL)
    vTaskDelete(handle);

  if (args != NULL)
    free(args);

  if (task->waiters == 0)
    *task = (Task){0};

  xSemaphoreGive(tasks_mutex);

  return 0;
}

bool task_is_finished(const char *id) {
  if (id == NULL || tasks_mutex == NULL)
    return true;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int index = find_id_locked(id);
  bool finished = index == -1 || tasks[index].status == FINISHED;

  xSemaphoreGive(tasks_mutex);

  return finished;
}

void task_wait_for(const char *id) {
  if (id == NULL || tasks_mutex == NULL)
    return;

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  int index = find_id_locked(id);

  if (index == -1 || tasks[index].status == FINISHED) {
    xSemaphoreGive(tasks_mutex);
    return;
  }

  tasks[index].waiters++;
  EventBits_t bit = task_bit(index);

  xSemaphoreGive(tasks_mutex);

  xEventGroupWaitBits(completion_events, bit, pdFALSE, pdFALSE, portMAX_DELAY);

  xSemaphoreTake(tasks_mutex, portMAX_DELAY);

  tasks[index].waiters--;

  if (tasks[index].status == FINISHED && tasks[index].waiters == 0)
    tasks[index] = (Task){0};

  xSemaphoreGive(tasks_mutex);
}
