#pragma once

#include "buzzer.h"
#include "icons.h"
#include "menu.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/missions.h"
#include "ui/screen.h"
#include "ui/views/mission.h"

#include <inttypes.h>
#include <stdio.h>

void missions_view(MissionType type);
