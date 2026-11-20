#include "effect.h"

char *effect_to_string(PetEffect effect) {
  switch (effect) {
  case PET_EFFECT_NONE:
    return NULL;
  case PET_EFFECT_WELL_FED:
    return "well_fed";
  case PET_EFFECT_HYDRATED:
    return "hydrated";
  case PET_EFFECT_HAPPY:
    return "happy";
  case PET_EFFECT_RESTED:
    return "rested";
  case PET_EFFECT_HUNGRY:
    return "hungry";
  case PET_EFFECT_DEHYDRATED:
    return "dehydrated";
  case PET_EFFECT_EXHAUSTED:
    return "exhausted";
  case PET_EFFECT_UNHAPPY:
    return "unhappy";
  case PET_EFFECT_INSOMNIA:
    return "insomnia";
  case PET_EFFECT_GLUTTONY:
    return "gluttony";
  case PET_EFFECT_DIRTY:
    return "dirty";
  case PET_EFFECT_COMMON_COLD:
    return "common_cold";
  case PET_EFFECT_STOMACH_BUG:
    return "stomach_bug";
  case PET_EFFECT_FEVER:
    return "fever";
  case PET_EFFECT_FLEAS:
    return "fleas";
  case PET_EFFECT_FOOD_POISONING:
    return "food_poisoning";
  case PET_EFFECT_FOOD_NIGHTMARES:
    return "food_nightmares";
  }
  return NULL;
}

PetEffect effect_from_string(const char *effect) {
  if (strcmp(effect, "well_fed") == 0)
    return PET_EFFECT_WELL_FED;
  if (strcmp(effect, "hydrated") == 0)
    return PET_EFFECT_HYDRATED;
  if (strcmp(effect, "happy") == 0)
    return PET_EFFECT_HAPPY;
  if (strcmp(effect, "rested") == 0)
    return PET_EFFECT_RESTED;
  if (strcmp(effect, "hungry") == 0)
    return PET_EFFECT_HUNGRY;
  if (strcmp(effect, "dehydrated") == 0)
    return PET_EFFECT_DEHYDRATED;
  if (strcmp(effect, "exhausted") == 0)
    return PET_EFFECT_EXHAUSTED;
  if (strcmp(effect, "unhappy") == 0)
    return PET_EFFECT_UNHAPPY;
  if (strcmp(effect, "insomnia") == 0)
    return PET_EFFECT_INSOMNIA;
  if (strcmp(effect, "gluttony") == 0)
    return PET_EFFECT_GLUTTONY;
  if (strcmp(effect, "dirty") == 0)
    return PET_EFFECT_DIRTY;
  if (strcmp(effect, "common_cold") == 0)
    return PET_EFFECT_COMMON_COLD;
  if (strcmp(effect, "stomach_bug") == 0)
    return PET_EFFECT_STOMACH_BUG;
  if (strcmp(effect, "fever") == 0)
    return PET_EFFECT_FEVER;
  if (strcmp(effect, "fleas") == 0)
    return PET_EFFECT_FLEAS;
  if (strcmp(effect, "food_poisoning") == 0)
    return PET_EFFECT_FOOD_POISONING;
  if (strcmp(effect, "food_nightmares") == 0)
    return PET_EFFECT_FOOD_NIGHTMARES;

  return PET_EFFECT_NONE;
}
