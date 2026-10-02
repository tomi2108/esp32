#include "melody.h"

#define RETURN_MELODY(name)                                                    \
  return (Melody){                                                             \
      .count = sizeof(name) / sizeof(Note),                                    \
      .notes = name,                                                           \
  };

#define DEFINE_MELODY(name)                                                    \
  Melody melody_##name() { RETURN_MELODY(name) }
#define DEFINE_SOUND(name)                                                     \
  Melody sound_##name() { RETURN_MELODY(name) }

uint32_t duration_to_ms(Duration duration, uint32_t whole_note) {
  if (duration > 0)
    return (whole_note) / duration;
  else if (duration < 0)
    return (whole_note) * 1.5 / abs(duration);
  else
    return 0;
};

Note happy_birthday[] = {
    {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_D4, D_QUARTER},
    {NOTE_C4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_E4, D_HALF},
    {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_D4, D_QUARTER},
    {NOTE_C4, D_QUARTER}, {NOTE_G4, D_QUARTER}, {NOTE_F4, D_HALF},
    {NOTE_C4, QUARTER},   {NOTE_C4, EIGHTH},    {NOTE_C5, D_QUARTER},
    {NOTE_A4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_E4, D_QUARTER},
    {NOTE_D4, D_QUARTER}, {NOTE_AS4, QUARTER},  {NOTE_AS4, EIGHTH},
    {NOTE_A4, D_QUARTER}, {NOTE_F4, D_QUARTER}, {NOTE_G4, D_QUARTER},
    {NOTE_F4, D_HALF},
};

static const Note boot[] = {
    {NOTE_C4, EIGHTH},
    {NOTE_E4, EIGHTH},
    {NOTE_G4, EIGHTH},
    {NOTE_C5, QUARTER},
};

static const Note disconnect[] = {
    {NOTE_G5, EIGHTH},
    {NOTE_E5, EIGHTH},
    {NOTE_C5, EIGHTH},
    {NOTE_G4, QUARTER},
};

static const Note error[] = {
    {NOTE_E4, EIGHTH},
    {NOTE_C4, EIGHTH},
    {NOTE_E4, QUARTER},
};

static const Note success[] = {
    {NOTE_C5, EIGHTH},
    {NOTE_E5, EIGHTH},
    {NOTE_G5, QUARTER},
};

static const Note warning[] = {
    {NOTE_A4, EIGHTH},
    {NOTE_A4, EIGHTH},
    {NOTE_A4, QUARTER},
};

static const Note click[] = {
    {NOTE_C5, EIGHTH},
};

static const Note notification[] = {
    {NOTE_E5, EIGHTH},
    {NOTE_G5, EIGHTH},
    {NOTE_E5, QUARTER},
};

DEFINE_MELODY(happy_birthday)
DEFINE_SOUND(boot)
DEFINE_SOUND(disconnect)
DEFINE_SOUND(error)
DEFINE_SOUND(success)
DEFINE_SOUND(warning)
DEFINE_SOUND(click)
DEFINE_SOUND(notification)
