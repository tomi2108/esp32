#pragma once

#include "client.h"
#include "json.h"

void api_buy_item(int32_t item_id);
int32_t api_publish_score(char *game_code, int32_t score);
