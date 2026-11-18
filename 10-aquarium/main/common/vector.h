#pragma once

#include <math.h>
#include <stdint.h>

typedef struct {
  float x;
  float y;
} Vector;

Vector vector_new(float x, float y);
bool vector_equals(Vector a, Vector b);
Vector vector_add(Vector a, Vector b);
Vector vector_sub(Vector a, Vector b);
float vector_dot(Vector a, Vector b);
Vector vector_scale(Vector a, float b);
float vector_magnitude(Vector a);
Vector vector_normal(Vector a);
