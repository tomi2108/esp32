#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <sys/types.h>

#define MAX_CHARACTER_SIZE 10

typedef const uint8_t FontData[96][MAX_CHARACTER_SIZE];

typedef struct Font {
  FontData data;
  uint8_t width;
  uint8_t height;
} Font;

uint32_t font_default(Font **out);
uint32_t font_eight_by_eight(Font **out);
