#pragma once

#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "icons.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "tictactoe.h"
#include "snake.h"

#define GAMES_MENU_LENGTH 2

void open_games_menu();
