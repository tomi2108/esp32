#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 10
#define TASK_STACK_SIZE 8192
#define TASK_PRIORITY 1

typedef void (*TaskHandler)(void *args);

typedef enum { FINISHED, RUNNING } TaskStatus;

typedef struct {
  TaskHandle_t handle;
  const char *id;
  TaskStatus status;
  void *args;
  TaskHandler handler;
} Task;

void task_manager_init(void);
int task_start(const char *id, TaskHandler handler, void *args);
int task_stop(const char *id);
