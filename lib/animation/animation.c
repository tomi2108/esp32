#include "animation.h"

Frame animation_frame(Animation *animation) {
  TickType_t now = xTaskGetTickCount();

  if (now - animation->last_frame_tick >= animation->frame_duration) {
    animation->current = (animation->current + 1) % animation->frame_count;
    animation->last_frame_tick = now;
  }

  return animation->frames[animation->current];
}
