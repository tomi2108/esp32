#pragma once

#include "common/task.h"

#include <inttypes.h>
#include <stdint.h>

int32_t wallet_get_coins(void);
int wallet_add_coins(int32_t coins);
int wallet_remove_coins(int32_t coins);
