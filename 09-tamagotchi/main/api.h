#pragma once

#include "client.h"
#include "json.h"

void api_buy_item(int32_t item_id);
Response *api_publish_score(char* game_code, int32_t score);
