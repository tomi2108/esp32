#include "analog.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
#include <math.h>

#define PI 3.14159265358979323846f

#define SPEAKER_GPIO GPIO_NUM_25

// #define BUFFER_SIZE 4096
// #define SAMPLE_RATE 44100

#define BUFFER_SIZE 400
#define SAMPLE_RATE 4000

extern const uint8_t song_wav_start[] asm("_binary_song_wav_start");
extern const uint8_t song_wav_end[] asm("_binary_song_wav_end");

typedef struct {
  float samples[BUFFER_SIZE];
  float frequency;
} Oscillator;

void sin_wave(Oscillator *o) {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    float t = (float)i / SAMPLE_RATE;
    o->samples[i] = sinf(2.0f * PI * o->frequency * t);
  }
}

void app_main(void) {
  analog_init();

  size_t song_size = song_wav_end - song_wav_start;
  const uint8_t *samples = song_wav_start + 44;
  size_t sample_count = song_size - 44;
  while (1) {
    for (int i = 0; i < sample_count; i++) {
      analog_write(SPEAKER_GPIO, samples[i]);
      esp_rom_delay_us(62);
    }
  }
}
