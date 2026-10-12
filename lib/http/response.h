#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "buffer.h"
#include "protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *status[1000];

typedef struct Response {
  uint16_t status_code;
  Headers headers;
  Body body;
} Response;

Response *response_create();
void response_destroy(Response *res);

#endif
