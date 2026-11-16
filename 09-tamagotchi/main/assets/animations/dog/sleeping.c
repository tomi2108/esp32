#include "sleeping.h"

const uint8_t dog_sleeping_frame_0[] = {};
const uint8_t dog_sleeping_frame_1[] = {};
const uint8_t dog_sleeping_frame_2[] = {};
const uint8_t dog_sleeping_frame_3[] = {};

static const Frame dog_sleeping_frames[] = {
    dog_sleeping_frame_0, dog_sleeping_frame_1, dog_sleeping_frame_2,
    dog_sleeping_frame_3};

Animation dog_sleeping = {
    .current = 0,
    .frame_count = 4,
    .width = 32,
    .height = 32,
    .frame_duration = pdMS_TO_TICKS(500),
    .last_frame_tick = 0,
    .frames = dog_sleeping_frames,
};
