#include "dates.h"

char *now_timestamp(void) {
  static char timestamp[32];
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm tm_info;
  gmtime_r(&ts.tv_sec, &tm_info);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &tm_info);

  snprintf(timestamp + 19, sizeof(timestamp) - 19, ".%03ldZ",
           (long)((ts.tv_nsec / 1000000) % 1000));
  return timestamp;
}
