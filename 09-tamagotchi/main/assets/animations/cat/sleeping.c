#include "sleeping.h"

const uint8_t cat_sleeping_frame_0[] = {};
const uint8_t cat_sleeping_frame_1[] = {};
const uint8_t cat_sleeping_frame_2[] = {};
const uint8_t cat_sleeping_frame_3[] = {};

static const Frame cat_sleeping_frames[] = {
    cat_sleeping_frame_0, cat_sleeping_frame_1, cat_sleeping_frame_2,
    cat_sleeping_frame_3};

Animation cat_sleeping = {
    .current = 0,
    .frame_count = 4,
    .width = 32,
    .height = 32,
    .frame_duration = pdMS_TO_TICKS(500),
    .last_frame_tick = 0,
    .frames = cat_sleeping_frames,
};
