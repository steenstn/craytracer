#include <math.h>

typedef struct Vector {
    float x;
    float y;
    float z;
} Vector;

static inline float vector_dot(Vector a, Vector b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline Vector vector_cross(Vector a, Vector b) {
    Vector res = {
        .x = a.y*b.z-a.z*b.y,
        .y = -1*(a.x*b.z-a.z*b.x),
        .z = a.x*b.y-a.y*b.x
    };

    return res;
}

static inline Vector vector_normalize(Vector v) {
    float abs = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    Vector normalized = {
        .x = v.x / abs,
        .y = v.y / abs,
        .z = v.z / abs
    };
    return normalized;
}

static inline Vector vector_plus(Vector a, Vector b) {
    Vector res = {
        .x = a.x+b.x,
        .y = a.y+b.y,
        .z = a.z+b.z
    };
    return res;
}

static inline Vector vector_multiply(Vector a, Vector b) {
    Vector res = {
        .x = a.x*b.x,
        .y = a.y*b.y,
        .z = a.z*b.z
    };
    return res;
}

static inline Vector vector_multiplyf(Vector v, float d) {
    Vector res = {
        .x = v.x*d,
        .y = v.y*d,
        .z = v.z*d
    };
    return res;
}

static inline Vector vector_minus(Vector a, Vector b) {
    Vector res = {
        .x = a.x-b.x,
        .y = a.y-b.y,
        .z = a.z-b.z
    };
    return res;
}

static inline Vector vector_dividef(Vector v, float f) {
    Vector res = {
        .x = v.x/f,
        .y = v.y/f,
        .z = v.z/f
    };
    return res;
}

static inline float vector_length(Vector v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}



