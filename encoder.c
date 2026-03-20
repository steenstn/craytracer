#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void encode(unsigned char charToEncode, FILE* file) {
  
  //printf("Encoding: %c\n", charToEncode);
  unsigned char block[8];
  if (fread(block, 1, 8, file) != 8) {
    printf("Failed to read block");
  }

  /*
  printf("Block: \n");
  for(int i = 0; i < 8; i++) {
      printf("%d ", block[i]);
  }
  printf("\n");
  */
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
/*
  printf("Block after fix: \n");
  for(int i = 0; i < 8; i++) {
      printf("%d ", block[i]);
  }
  printf("\n");
  */

    fwrite(block, 1, 8, file); 
}

bool encode_message(void* data, size_t data_size, FILE* file) {
    unsigned char header[14];
    size_t res = fread(header, 14, 1, file);
    printf("%zu", res);

    if (header[0] != 'B' || header[1] != 'M') {
        printf("File is not a BMP file, aborting\n");
        return false;
    }

    unsigned char image_start = header[10];
    printf("Image starts at offset %d\n", image_start);
    fseek(file, image_start, SEEK_SET);


    printf("Encoding data size: %zu\n", data_size);
    // Check data size, encode bit shifted size
    // Read must check size of size_t
    encode((unsigned char)(data_size>>(3*8)), file);
    encode((unsigned char)(data_size>>(2*8)), file);
    encode((unsigned char)(data_size>>8), file);
    encode((unsigned char)data_size, file);

    for(int i = 0; i < data_size; i++) {
        encode(((unsigned char*)data)[i], file);
    }

    return true;
}

static unsigned char decode(FILE* file) {
    unsigned char block[8];
    if (fread(block, 1, 8, file) != 8) {
        printf("Failed to read block\n");
        return 0;
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
    return result;
}


void* decode_message(FILE* file) {
    unsigned char header[14];

    if (fread(header, 14, 1, file) != 1) {
        printf("Failed to read header\n");
        return NULL;
    }
    unsigned char imageStart = header[10];

    fseek(file, imageStart, SEEK_SET);
    size_t messageSize = ((size_t)decode(file)<<(3*8)) + ((size_t)decode(file)<<(2*8)) + ((size_t)decode(file)<<(1*8)) + (size_t)decode(file);
    printf("decoded message size: %zu\n", messageSize);

    //unsigned char result = decode(file);

    //unsigned int messageSize = (int)result;

    void* result_pointer = malloc(messageSize);
    printf("Encoded message size: %zu\n\n", messageSize);
    for(int i = 0; i < messageSize; i++) {
        unsigned char result = decode(file);

        ((unsigned char*)result_pointer)[i] = result;
    }

    return result_pointer;
}
