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

void save_bmp(const char *filename, const unsigned char *image_data, int w, int h) {

  int i;
  FILE *f;
  int filesize = 54 + 3 * w * h;

  unsigned char bmpfileheader[14] = {'B', 'M', 0, 0,  0, 0, 0,
    0,   0,   0, 54, 0, 0, 0};
  unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0,  0,
    0,  0, 0, 0, 1, 0, 24, 0};
  unsigned char bmppad[3] = {0, 0, 0};

  bmpfileheader[2] = (unsigned char)(filesize);
  bmpfileheader[3] = (unsigned char)(filesize >> 8);
  bmpfileheader[4] = (unsigned char)(filesize >> 16);
  bmpfileheader[5] = (unsigned char)(filesize >> 24);

  bmpinfoheader[4] = (unsigned char)(w);
  bmpinfoheader[5] = (unsigned char)(w >> 8);
  bmpinfoheader[6] = (unsigned char)(w >> 16);
  bmpinfoheader[7] = (unsigned char)(w >> 24);
  bmpinfoheader[8] = (unsigned char)(h);
  bmpinfoheader[9] = (unsigned char)(h >> 8);
  bmpinfoheader[10] = (unsigned char)(h >> 16);
  bmpinfoheader[11] = (unsigned char)(h >> 24);

  // f = fopen("img.raw","wb");
  // fwrite(img,3,w*h,f);
  // fclose(f);

  // Make the int-array into a char-array for the bmp
  /*int counter = 0;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      img[counter] = (unsigned char)picture[j][i][2];
      img[counter + 1] = (unsigned char)picture[j][i][1];
      img[counter + 2] = (unsigned char)picture[j][i][0];
      counter = counter + 3;
    }
  }*/

  unsigned char temp[w*h*3];
  for(int i = 0; i < w*h*3;i+=3) {
      temp[i]=image_data[i+2];
      temp[i+1]=image_data[i+1];
      temp[i+2]=image_data[i];
  }

  f = fopen(filename, "wb+");
  fwrite(bmpfileheader, 1, 14, f);
  fwrite(bmpinfoheader, 1, 40, f);

  for (i = 0; i < h; i++) {
    fwrite(temp + (w * (h - i - 1) * 3), 3, w, f);
    fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
  }
  fclose(f);
}
