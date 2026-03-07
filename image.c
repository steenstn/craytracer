#include <stdio.h>
#include <stdbool.h>

bool save_image(unsigned char* image, char* path, int width, int height) {
    FILE *fptr = fopen(path, "w");
    if (fptr == NULL) {
        return false;
    }
    fprintf(fptr, "P3\n");
    fprintf(fptr, "%d %d\n", width, height);
    fprintf(fptr, "255\n");
    int index = 0;
    for(int h = 0; h < height; h++) {
        for(int w= 0; w < width; w++) {
            for(int i = 0; i < 3; i++) {
                fprintf(fptr, " %d ", image[index++]);
            }
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
    return true;
}
