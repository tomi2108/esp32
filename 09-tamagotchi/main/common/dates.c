#include "dates.h"

char *now_timestamp(void) {
  static char timestamp[25];
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm *tm_info = gmtime(&ts.tv_sec);
  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           ts.tv_nsec / 1000000);
  return timestamp;
}
