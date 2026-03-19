#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void encode(unsigned char charToEncode, FILE* file) {
  
  printf("Encoding: %c\n", charToEncode);
  unsigned char block[8];
  size_t res = fread(block, 1, 8, file);

  printf("Block: \n");
  for(int i = 0; i < 8; i++) {
      printf("%d ", block[i]);
  }
  printf("\n");
  printf("res size %zu\n", res);
  fseek(file, -8, SEEK_CUR); 
  
  unsigned char target[8] =
      {
        (charToEncode & 0b1),
        (charToEncode & 0b10) >> 1,
        (charToEncode & 0b100) >> 2,
        (charToEncode & 0b1000) >> 3,
        (charToEncode & 0b10000) >> 4,
        (charToEncode & 0b100000) >> 5,
        (charToEncode & 0b1000000) >> 6,
        (charToEncode & 0b10000000) >> 7
      };

 for(int j = 0; j < 8; j++) {
      if(target[j] > (block[j] & 0b1)) {
        block[j]++;
      } else if (target[j] < (block[j] & 0b1)) {
        block[j]--;
      }
    }

  printf("Block after fix: \n");
  for(int i = 0; i < 8; i++) {
      printf("%d ", block[i]);
  }
  printf("\n");

    fwrite(block, 1, 8, file); 
}

bool encode_message(void* data, size_t data_size, FILE* file) {
    unsigned char header[14];
    size_t res = fread(header, 14, 1, file);
    printf("%zu", res);

    if (header[0] != 'B' && header[1] != 'M') {
        printf("File is not a BMP file, aborting\n");
        return false;
    }

    unsigned char image_start = header[10];
    printf("Image starts at offset %d\n", image_start);
    fseek(file, image_start, SEEK_SET);

    encode((unsigned char)data_size, file);
    for(int i = 0; i < data_size; i++) {
        encode(((unsigned char*)data)[i], file);
    }

    return true;
}

void* decode_message(FILE* file) {
    unsigned char header[14];

    if (fread(header, 14, 1, file) != 1) {
        printf("Failed to read header\n");
        return NULL;
    }
    unsigned char imageStart = header[10];


    fseek(file, imageStart, SEEK_SET);
    unsigned char block[8];
    if (fread(block, 1, 8, file) != 8) {
        printf("Failed to read block\n");
        return NULL;
    }
    unsigned char result =
      (block[0] & 0b1) +
      ((block[1] & 0b1) << 1) +
      ((block[2] & 0b1) << 2) +
      ((block[3] & 0b1) << 3) +
      ((block[4] & 0b1) << 4) +
      ((block[5] & 0b1) << 5) +
      ((block[6] & 0b1) << 6) +
      ((block[7] & 0b1) << 7);

    unsigned int messageSize = (int)result;
    void* result_pointer = malloc(messageSize);
    printf("Encoded message size: %d\n\n", messageSize);
    for(int i = 0; i < messageSize; i++) {
        if (fread(block, 1, 8, file) != 8) {
            free(result_pointer);
            return NULL;
        }
        result =
          (block[0] & 0b1) +
          ((block[1] & 0b1) << 1) +
          ((block[2] & 0b1) << 2) +
          ((block[3] & 0b1) << 3) +
          ((block[4] & 0b1) << 4) +
          ((block[5] & 0b1) << 5) +
          ((block[6] & 0b1) << 6) +
          ((block[7] & 0b1) << 7);

        //memcpy(&result_pointer[i], &result, 1);
        ((unsigned char*)result_pointer)[i] = result;
        //printf("%c", result);
    }

    //printf("\n");
    return result_pointer;
}
