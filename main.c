#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <omp.h>

#include "util.c"
#include "image.c"
#include "vector.c"
#include "raytracer.c"

#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 768

/// https://gabrielgambetta.com/computer-graphics-from-scratch/05-extending-the-raytracer.html

// For the file
unsigned char image_data[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

float picture[IMAGE_WIDTH*IMAGE_HEIGHT*3];

const int numRays = 1; // Rays per iteration

#define NUM_SPHERES 100
Sphere all_spheres[NUM_SPHERES];
Vector all_colors[NUM_SPHERES];

void print_vector(Vector v) {
    printf("x: %f y: %f z: %f\n", v.x, v.y, v.z);
}

int main(void) {

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize: %s\n", SDL_GetError());
        return 1;
    };
    SDL_Window* window = SDL_CreateWindow("C Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, IMAGE_WIDTH, IMAGE_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);

    SDL_Event e;
    bool quit = false;

    srand(omp_get_wtime());
    Vector white = (Vector){1.0,1.0,1.0};
    Vector color_1 = (Vector){0.6,0.1,0.4};
    Vector color_2 = (Vector){0.3,0.1,0.8};
    color_1 = vector_dividef(vector_plus(color_1, white), 2);
    color_2 = vector_dividef(vector_plus(color_2, white), 2);

    all_spheres[0] = (Sphere){.position = {.y=-1, .z=-60}, .radius=1.0};
    all_colors[0] = color_1;
    all_colors[1] = color_1;
    all_colors[2] = color_1;
    all_spheres[1] = (Sphere){.position = {.x=6*make_random()-3, .y=-1, .z=-65}, .radius=1.0};
    all_spheres[2] = (Sphere){.position = {.x=7, .y=-1, .z=-50}, .radius=1.0};


    for(int i = 3; i < NUM_SPHERES-1; i++) {
        int sphere_to_grow_from_index = floor(make_random()*i);

        Vector direction = (Vector){2*make_random()-1, make_random()*-1, 2*make_random()-1};
        direction = vector_normalize(direction);
        Vector new_position = vector_plus(all_spheres[sphere_to_grow_from_index].position, vector_multiplyf(direction,6.0));
        Vector the_color = make_random() > 0.6 ? color_1 : color_2;
        all_spheres[i] = (Sphere){.position = new_position, .radius = 2.0};
        all_colors[i] = the_color;
    }

    all_colors[NUM_SPHERES-1] = (Vector){.x = 0.79, .y = 0.79, .z = 0.79};
    all_spheres[NUM_SPHERES-1] = (Sphere){.position = {.y=10000, .z=-60}, .radius=10000};
    
    Vector s = {0,-1,0};
    float xmax = 5, ymax = 5;
    int num_passes = 0;
    while(quit == false) {

        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        s = vector_plus(s, (Vector){.z=-0.5});
                        break;
                    case SDLK_s:
                        s = vector_plus(s, (Vector){.z=0.5});
                        break;
                    case SDLK_a:
                        s = vector_plus(s, (Vector){.x=-0.5});
                        break;
                    case SDLK_d:
                        s = vector_plus(s, (Vector){.x=0.5});
                        break;
                    case SDLK_UP:
                        s = vector_plus(s, (Vector){.y=-0.5});
                        break;
                    case SDLK_DOWN:
                        s = vector_plus(s, (Vector){.y=0.5});
                        break;
                    case SDLK_p:
                        save_bmp("result.bmp", image_data, IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;
                        

                }
                    
                num_passes=0;
                memset(picture, 0, sizeof(picture));
                }
            
        }
        //double start = omp_get_wtime();

        #pragma omp parallel for collapse(2) schedule(dynamic, 16)
        for(int screenY = 0; screenY < IMAGE_HEIGHT; screenY++) {
            for(int screenX = 0; screenX < IMAGE_WIDTH; screenX++) {
                Vector end_color = {};
                float x, y;
                x = (float)(screenX * 6) / (float)IMAGE_WIDTH - 3.0;
                y = ((float)(screenY * 6) / (float)IMAGE_HEIGHT - 3.0) * ((float)IMAGE_HEIGHT / (float)IMAGE_WIDTH);

                Vector dir =  {.x = x / xmax, .y = y / ymax, .z = -1};
                dir = vector_normalize(dir);

                for(int i = 0; i < numRays; i++) {
                    // bounces = 0
                    
                    end_color = vector_plus(end_color, shoot_ray(all_spheres, all_colors, NUM_SPHERES, s, dir));
                    //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);
                }
                end_color = vector_dividef(end_color, (float)numRays);
                //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);

                int pixelIndex = (screenX + screenY * IMAGE_WIDTH) * 3;
                picture[pixelIndex] +=end_color.x;
                picture[pixelIndex+1] +=end_color.y;
                picture[pixelIndex+2] +=end_color.z;

            }


        }
        //double end = omp_get_wtime();
        //double time_taken = end-start;
        //printf("Time: %f\n", time_taken);
        num_passes++;
        #pragma omp parallel for
        for(int i = 0; i < IMAGE_WIDTH*IMAGE_HEIGHT*3; i+=3) {
                float r = picture[i];
                float g = picture[i+1];
                float b = picture[i+2];
                float inv_passes = 255.0f/num_passes;
                short end_r = clamp(r* inv_passes, 0, 255);
                short end_g = clamp(g* inv_passes, 0, 255);
                short end_b = clamp(b* inv_passes, 0, 255);
                image_data[i] = (unsigned char)end_r;
                image_data[i+1] = (unsigned char)end_g;
                image_data[i+2] = (unsigned char)end_b;
        }
        
        void* locked_pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &locked_pixels, &pitch);

        memcpy(locked_pixels, image_data, IMAGE_WIDTH*IMAGE_HEIGHT*3);

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);


    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

