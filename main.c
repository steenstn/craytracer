#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <time.h>

#define NUM_SPHERES 1000
#include "util.c"
#include "image.c"
#include "vector.c"
#include "raytracer.c"

#define IMAGE_WIDTH 1600
#define IMAGE_HEIGHT 1200

/// https://gabrielgambetta.com/computer-graphics-from-scratch/05-extending-the-raytracer.html

// For the file
unsigned char image_data[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

float picture[IMAGE_WIDTH*IMAGE_HEIGHT*3];

const int numRays = 1; // Rays per iteration

typedef struct Scene {
    Sphere all_spheres[NUM_SPHERES];
    Vector all_colors[NUM_SPHERES];
    Vector s;
} Scene;

void print_vector(Vector v) {
    printf("x: %f y: %f z: %f\n", v.x, v.y, v.z);
}



int main(int argc, char* argv[]) {

    state = (XorShiftR128PlusState){.s = {time(NULL), time(NULL)/2}};
    Scene scene;

    printf("Size of scene: %zu\n", sizeof scene);
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

    //srand(omp_get_wtime());

    if (argc == 2) {
        printf("Loading %s\n", argv[1]);
        FILE *file;
        file = fopen("result.bmp", "rb");
        fseek(file, 50, SEEK_SET);
        size_t res = fread(&scene, sizeof(scene), 1, file);
        printf("Read %zu\n", res);
        fclose(file);
    } else {
        Vector white = (Vector){1.0,1.0,1.0};
        Vector color_1 = (Vector){0.6,0.1,0.4};
        Vector color_2 = (Vector){0.3,0.1,0.8};
        color_1 = vector_dividef(vector_plus(color_1, white), 2);
        color_2 = vector_dividef(vector_plus(color_2, white), 2);

        int num_start_spheres = 10;

        for(int i = 0; i < num_start_spheres; i++) {
            scene.all_spheres[i] = (Sphere){.position = {.x = 100*make_random()-50, .y=-1, .z=-70 + 20*make_random()}, .radius=1.0};
            scene.all_colors[i] = color_1;
        }


        for(int i = num_start_spheres; i < NUM_SPHERES-1; i++) {
            int sphere_to_grow_from_index = floor(make_random()*i);

            Vector direction = (Vector){2*make_random()-1, make_random()*-1, 2*make_random()-1};
            direction = vector_normalize(direction);
            Vector new_position = vector_plus(scene.all_spheres[sphere_to_grow_from_index].position, vector_multiplyf(direction,6.0));
            Vector the_color = make_random() > 0.6 ? color_1 : color_2;
            scene.all_spheres[i] = (Sphere){.position = new_position, .radius = 2.0};
            scene.all_colors[i] = the_color;
        }

        // Big ground sphere
        scene.all_colors[NUM_SPHERES-1] = (Vector){.x = 0.79, .y = 0.79, .z = 0.79};
        scene.all_spheres[NUM_SPHERES-1] = (Sphere){.position = {.y=10000, .z=-60}, .radius=10000};
        
        scene.s = (Vector){0,-3,20};
    }

    //Vector target = {1, 0, -40};
    float xmax = 5, ymax = 5;
    int num_passes = 0;
    int step = 2;
    FILE *file;
    while(quit == false) {

        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        scene.s = vector_plus(scene.s, (Vector){.z=-0.5});
                    break;
                    case SDLK_s:
                        scene.s = vector_plus(scene.s, (Vector){.z=0.5});
                    break;
                    case SDLK_a:
                        scene.s = vector_plus(scene.s, (Vector){.x=-0.5});
                    break;
                    case SDLK_d:
                        scene.s = vector_plus(scene.s, (Vector){.x=0.5});
                    break;
                    case SDLK_UP:
                        scene.s = vector_plus(scene.s, (Vector){.y=-0.5});
                    break;
                    case SDLK_DOWN:
                        scene.s = vector_plus(scene.s, (Vector){.y=0.5});
                    break;
                    case SDLK_q:
                        step = step <=1 ? 1 : step/2;
                        printf("Step: %d\n", step);
                    break;
                    case SDLK_e:
                        step = step >=16 ? 16 : step*2;
                        printf("Step: %d\n", step);
                    break;
                    case SDLK_p:
                        save_bmp("result.bmp", image_data, IMAGE_WIDTH, IMAGE_HEIGHT);
                    break;
                    case SDLK_b:
                        file = fopen("result.bmp", "rb+");
                        if (file) {
                            fseek(file,50, SEEK_SET);
                            fwrite(&scene, sizeof(scene), 1, file);
                            fclose(file);
                        }
                    break;

                }
                    
                num_passes=0;
                memset(picture, 0, sizeof(picture));
                }
            
        }

        double start = omp_get_wtime();

        #pragma omp parallel for collapse(2) schedule(dynamic, 16)
        for(int screenY = 0; screenY < IMAGE_HEIGHT; screenY+=step) {
            for(int screenX = 0; screenX < IMAGE_WIDTH; screenX+=step) {
                Vector end_color = {};
                float x, y;
                x = (float)(screenX * 6) / (float)IMAGE_WIDTH - 3.0;
                y = ((float)(screenY * 6) / (float)IMAGE_HEIGHT - 3.0) * ((float)IMAGE_HEIGHT / (float)IMAGE_WIDTH);
                //Vector dadir = vector_minus(target, s)
                Vector dir =  {.x = x / xmax, .y = y / ymax, .z = -1};
                dir = vector_normalize(dir);

                for(int i = 0; i < numRays; i++) {
                    // bounces = 0
                    
                    end_color = vector_plus(end_color, shoot_ray(scene.all_spheres, scene.all_colors, NUM_SPHERES, scene.s, dir));
                    //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);
                }
                end_color = vector_dividef(end_color, (float)numRays);
                //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);
                
                for(int xx = 0; xx < step; xx++) {
                    for(int yy = 0; yy < step; yy++) {
                        int pixelIndex = (screenX+xx + (screenY+yy) * IMAGE_WIDTH) * 3;
                        picture[pixelIndex] +=end_color.x;
                        picture[pixelIndex+1] +=end_color.y;
                        picture[pixelIndex+2] +=end_color.z;
                    }
                }

            }


        }
        double end = omp_get_wtime();
        double time_taken = end-start;
        printf("Time: %f\n", time_taken);
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

