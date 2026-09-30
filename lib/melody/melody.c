#include "melody.h"

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

Note new_message[] = {
    {NOTE_C5, EIGHTH}, {NOTE_E5, EIGHTH}, {NOTE_G5, QUARTER},
    {NOTE_E5, EIGHTH}, {NOTE_G5, EIGHTH}, {NOTE_B5, QUARTER},
    {NOTE_G5, EIGHTH}, {NOTE_E5, EIGHTH}, {NOTE_C5, HALF},
};

Melody melody_happy_birthday() {
  return (Melody){.count = sizeof(happy_birthday) / sizeof(Note),
                  .notes = happy_birthday};
}

Melody melody_new_message() {
  return (Melody){.count = sizeof(new_message) / sizeof(Note),
                  .notes = new_message};
}

uint32_t duration_to_ms(Duration duration, uint32_t whole_note) {
  if (duration > 0)
    return (whole_note) / duration;
  else if (duration < 0)
    return (whole_note) * 1.5 / abs(duration);
  else
    return 0;
};
