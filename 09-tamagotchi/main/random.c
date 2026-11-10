#include "random.h"

int random_range(int min, int max) {
  return min + (esp_random() % (max - min + 1));
}
