#include <stdlib.h>

float make_random(void) { return (float)rand() / (float)RAND_MAX; }

float clamp(float x, float min, float max) { 
  return (x < min ? min : (x > max ? max : x));
}

float clampint(int x, int min, int max) { 
  return (x < min ? min : (x > max ? max : x));
}
