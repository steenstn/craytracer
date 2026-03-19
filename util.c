#include <stdint.h>
#include <string.h>

//float make_random(void) { return (float)rand() / (float)RAND_MAX; }

float clamp(float x, float min, float max) { 
  return (x < min ? min : (x > max ? max : x));
}

float clampint(int x, int min, int max) { 
  return (x < min ? min : (x > max ? max : x));
}

typedef struct {
    uint64_t s[2]; // seeds
} XorShiftR128PlusState;

// The state must be seeded so that it is not all zero
uint64_t xorshiftr128plus(XorShiftR128PlusState* state) {
	uint64_t x = state->s[0];
	const uint64_t y = state->s[1];
	state->s[0] = y;
	x ^= x << 23; // shift & xor
	x ^= x >> 17; // shift & xor
	x ^= y; // xor
	state->s[1] = x + y;
	return x;
}


// Global random state
XorShiftR128PlusState state = {};

float make_random() {

    uint32_t r = xorshiftr128plus(&state);
    uint32_t bits = (r >> 9) | 0x3F800000;
    float f;
    memcpy(&f, &bits, sizeof(f));  
    f -= 1.0f;
    return f;
}
