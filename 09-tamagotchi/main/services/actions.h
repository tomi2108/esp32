#pragma once

#include "buffer.h"
#include "common/task.h"
#include "json.h"
#include "mqttt.h"

void action_sleep();
void action_wakeup();
void action_shower();
void action_hydrate();
void action_consume(int item_id);
void action_equip(int item_id);
