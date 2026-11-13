#pragma once

#include "client.h"
#include "common/dates.h"
#include "common/task.h"
#include "json.h"
#include "response.h"
#include "services/actions.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MISSIONS_SIZE 30

typedef enum { MISSIONS_AUTOMATIC, MISSIONS_MANUAL } MissionType;

typedef struct {
  int32_t id;
  char *code;
  char *title;
  char *description;
  char *type;
  int32_t target;
  int32_t progress;
  int32_t reward_coins;
  char *completed_at;
} Mission;

void missions_refresh_async(void);
Mission *missions_get(MissionType type);
int missions_get_size(MissionType type);
void missions_complete(int32_t mission_id);
