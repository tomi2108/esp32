#pragma once

#include "client.h"
#include "common/task.h"
#include "json.h"
#include "response.h"

#include <inttypes.h>
#include <stdint.h>

void wallet_refresh_async(void);
int32_t wallet_get_coins(void);
