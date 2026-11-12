#pragma once

#include "buzzer.h"
#include "icons.h"
#include "menu.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/actions.h"
#include "ui/overlay.h"
#include "ui/views/main_menu.h"
#include "ui/views/shop.h"

#define TOOLBAR_LENGTH 3

void toolbar_render();
void toolbar_update();
