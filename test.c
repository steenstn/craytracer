
#include "encoder.c"
#include <stdio.h>

typedef struct DaStruct {
    float x;
    float y;
    float z;
    int a;
    int b;
} DaStruct;

int main() {

    FILE* file = fopen("result.bmp", "rb+");
    DaStruct yay = {.x = 2.5, .y = 200.23, .z = 1, .a = -5., .b = 33};

    encode_message(&yay, sizeof (yay), file);
    fclose(file);

    file = fopen("result.bmp", "rb");
    DaStruct* result = (DaStruct*)decode_message(file);

    printf("x: %f\n", result->x);
    printf("y: %f\n", result->y);
    printf("z: %f\n", result->z);
    printf("a: %d\n", result->a);
    printf("b: %d\n", result->b);
    return 0;
}
