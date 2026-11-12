#pragma once

#include "client.h"
#include "inbox.h"
#include "json.h"
#include "protocol.h"
#include <stdint.h>

void api_buy_item(int32_t item_id);
int32_t api_publish_score(char *game_code, int32_t score);
