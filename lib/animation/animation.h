#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef const uint8_t *Frame;

typedef struct Animation {
  int32_t current;
  int32_t frame_count;
  int32_t width;
  int32_t height;
  TickType_t frame_duration;
  TickType_t last_frame_tick;
  const Frame *frames;
} Animation;

Frame animation_frame(Animation *animation);

#endif
