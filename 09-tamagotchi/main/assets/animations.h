#pragma once

#include "animation.h"
#include "assets/animations/cat/idle.h"
#include "assets/animations/cat/sleeping.h"
#include "assets/animations/dog/idle.h"
#include "assets/animations/dog/sleeping.h"
#include "assets/animations/rabbit/idle.h"
#include "assets/animations/rabbit/sleeping.h"
#include "freertos/projdefs.h"
#include "services/state.h"
#include <string.h>

Animation *animations_pet_status(PetSpecies species, PetStatus status);
