#pragma once

#include "buzzer.h"
#include "icons.h"
#include "menu.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/actions.h"
#include "services/inventory.h"
#include "services/missions.h"
#include "ui/screen.h"
#include "ui/views/games.h"
#include "ui/views/inbox.h"
#include "ui/views/inventory.h"
#include "ui/views/missions.h"
#include "ui/views/shop.h"

#define MAIN_MENU_LENGTH 9

void main_menu_view(void);
