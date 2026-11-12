#pragma once

#include "buzzer.h"
#include "common/random.h"
#include "common/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "icons.h"
#include "json.h"
#include "menu.h"
#include "modal.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/games.h"
#include "services/wallet.h"
#include "ui/components/spinner.h"
#include "ui/overlay.h"
#include "ui/screen.h"

void tictactoe_play();
