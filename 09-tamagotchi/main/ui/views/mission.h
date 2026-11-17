#pragma once

#include "buzzer.h"
#include "icons.h"
#include "menu.h"
#include "oled.h"
#include "rotary_encoder.h"
#include "services/missions.h"
#include "ui/screen.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

void mission_view(MissionType type, int32_t mission_id);
