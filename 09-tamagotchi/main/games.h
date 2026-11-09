#pragma once

#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "icons.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "tictactoe.h"

#define GAMES_MENU_LENGTH 1

void open_games_menu();
