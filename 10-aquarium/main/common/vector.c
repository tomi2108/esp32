#include "vector.h"

Vector vector_new(float x, float y) { return (Vector){.x = x, .y = y}; }
bool vector_equals(Vector a, Vector b) { return a.x == b.x && a.y == b.y; }

Vector vector_add(Vector a, Vector b) {
  return (Vector){.x = a.x + b.x, .y = a.y + b.y};
}

Vector vector_sub(Vector a, Vector b) {
  return (Vector){.x = a.x - b.x, .y = a.y - b.y};
}

float vector_dot(Vector a, Vector b) { return a.x * b.x + a.y * b.y; }

Vector vector_scale(Vector a, float b) {
  return (Vector){.x = a.x * b, .y = a.y * b};
}

float vector_magnitude(Vector a) { return sqrt(a.x * a.x + a.y * a.y); }

Vector vector_normal(Vector a) {
  return vector_scale(a, 1.0f / vector_magnitude(a));
}
