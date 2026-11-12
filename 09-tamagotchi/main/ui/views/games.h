#pragma once

#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "games/snake.h"
#include "games/tictactoe.h"
#include "icons.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "ui/screen.h"

#define GAMES_MENU_LENGTH 2

void games_view(void);
