#include "animations.h"

Animation *animations_pet_status(PetSpecies species, PetStatus status) {
  switch (species) {
  case PET_CAT:
    switch (status) {
    case PET_IDLE:
      return &cat_idle;
    case PET_SLEEPING:
      return &cat_sleeping;
    }
    break;
  case PET_DOG:
    switch (status) {
    case PET_IDLE:
      return &dog_idle;
    case PET_SLEEPING:
      return &dog_sleeping;
    }
    break;
  case PET_RABBIT:
    switch (status) {
    case PET_IDLE:
      return &rabbit_idle;
    case PET_SLEEPING:
      return &rabbit_sleeping;
    }
  };
  return &cat_idle;
};
