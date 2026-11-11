#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define MAX_TASKS 10
#define TASK_STACK_SIZE 8192
#define TASK_PRIORITY 2

typedef void (*TaskHandler)(void *args);
typedef enum { FINISHED, RUNNING } TaskStatus;
typedef struct {
  TaskHandle_t handle;
  const char *id;
  TaskStatus status;
  void *args;
} Task;

int task_start(const char *id, TaskHandler handler, void *args);
int task_stop(const char *id);
