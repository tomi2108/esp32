#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <sys/types.h>

#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988

typedef enum Duration {
  WHOLE = 1,
  HALF = 2,
  QUARTER = 4,
  EIGHTH = 8,

  D_WHOLE = -1,
  D_HALF = -2,
  D_QUARTER = -4,
  D_EIGHTH = -8,
} Duration;

typedef struct Note {
  uint32_t frequency;
  Duration duration;
} Note;

uint32_t melody_happy_birthday(Note **out);
uint32_t duration_to_ms(Duration duration, uint32_t whole_note);
