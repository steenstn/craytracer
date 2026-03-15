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

#include "image.c"
#include "vector.c"

typedef struct ObjectHit {
    Vector position;
    int index;
} ObjectHit;

typedef struct Sphere {
    Vector position;
    float radius;
} Sphere;

#define IMAGE_WIDTH 1600
#define IMAGE_HEIGHT 1200
#define PIXEL_INDEX(x, y) ((x + y * IMAGE_WIDTH) * 3)
// For the file
unsigned char image_data[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

float picture[IMAGE_WIDTH*IMAGE_HEIGHT*3];

const int numRays = 1; // Rays per iteration

#define NUM_SPHERES 100
Sphere all_spheres[NUM_SPHERES];
Vector all_colors[NUM_SPHERES];

float make_random(void) { return (float)rand() / (float)RAND_MAX; }

float clamp(float x, float min, float max) { 
  return (x < min ? min : (x > max ? max : x));
}

float clampint(int x, int min, int max) { 
  return (x < min ? min : (x > max ? max : x));
}

Vector shoot_ray(Sphere *spheres, int num_spheres, Vector start, Vector direction);
ObjectHit ray_sphere_intersection(Sphere *spheres, int num_spheres, Vector start, Vector direction);

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

    all_spheres[0] = (Sphere){.position = {.z=-60}, .radius=1.0};
    all_colors[0] = color_1;
    all_colors[1] = color_1;
    all_colors[2] = color_1;
    all_spheres[1] = (Sphere){.position = {.x=-7, .y=-5, .z=-65}, .radius=1.0};
    all_spheres[2] = (Sphere){.position = {.x=7, .y=7, .z=-50}, .radius=1.0};


    for(int i = 3; i < NUM_SPHERES; i++) {
        int sphere_to_grow_from_index = floor(make_random()*i);

        Vector direction = (Vector){2*make_random()-1, 2*make_random()-1, 2*make_random()-1};
        direction = vector_normalize(direction);
        Vector new_position = vector_plus(all_spheres[sphere_to_grow_from_index].position, vector_multiplyf(direction,3.0));
        Vector the_color = make_random() > 0.6 ? color_1 : color_2;
        all_spheres[i] = (Sphere){.position = new_position, .radius = 2.0};
        all_colors[i] = the_color;
    }
    
    Vector s = {0,0,0};
    float xmax = 5, ymax = 5;
    int num_passes = 0;
    while(quit == false) {


        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        s = vector_plus(s, (Vector){.z=-0.5});
                        break;
                    case SDLK_DOWN:
                        s = vector_plus(s, (Vector){.z=0.5});
                        break;
                    case SDLK_LEFT:
                        s = vector_plus(s, (Vector){.x=-0.5});
                        break;
                    case SDLK_RIGHT:
                        s = vector_plus(s, (Vector){.x=0.5});
                        break;
                }
                    
                num_passes=0;
                memset(picture, 0, sizeof(picture));
                }
            
        }
        //double start = omp_get_wtime();

        //printf("New pass\n");
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
                    
                    end_color = vector_plus(end_color, shoot_ray(all_spheres, NUM_SPHERES, s, dir));
                    //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);
                }
                end_color = vector_dividef(end_color, (float)numRays);
                //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);

                picture[PIXEL_INDEX(screenX, screenY)] +=end_color.x;
                picture[PIXEL_INDEX(screenX, screenY)+1] +=end_color.y;
                picture[PIXEL_INDEX(screenX, screenY)+2] +=end_color.z;

            }


        }
        //double end = omp_get_wtime();
        //double time_taken = end-start;
        //printf("Time: %f\n", time_taken);
        num_passes++;
        for(int i = 0; i < IMAGE_WIDTH*IMAGE_HEIGHT*3; i+=3) {

                float r = picture[i];
                float g = picture[i+1];
                float b = picture[i+2];
                short end_r = clamp(r* 255/num_passes, 0, 255);
                short end_g = clamp(g* 255/num_passes, 0, 255);
                short end_b = clamp(b* 255/num_passes, 0, 255);
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

Vector shoot_ray(Sphere *spheres, int num_spheres, Vector start, Vector direction) {
  int max_bounces = 5; 
  Vector env = {1, 1, 1}; 
  Vector resulting_color = {};
  Vector throughput = {1.0, 1.0, 1.0};

  for(int num_bounces = 0; num_bounces < max_bounces; num_bounces++) {
    ObjectHit hit = ray_sphere_intersection(all_spheres, num_spheres, start, direction);

    if (hit.index == -1) {
        resulting_color = vector_plus(resulting_color, vector_multiply(throughput, env));

        break;
    }

    Sphere hit_sphere = all_spheres[hit.index];
   
    Vector hit_normal = vector_minus(hit.position, hit_sphere.position);
    hit_normal = vector_normalize(hit_normal);
    Vector tangent;

    if (fabs(hit_normal.x) > 0.1)
        tangent = (Vector){0,1,0};
    else
        tangent = (Vector){1,0,0};

    Vector bitangent = vector_normalize(vector_cross(hit_normal, tangent));
    tangent = vector_normalize(vector_cross(bitangent, hit_normal));
    float eps1 = make_random() * 6.28318; // 2*PI
    float eps2 = sqrtf(make_random());

    float x = cosf(eps1) * eps2;
    float y = sinf(eps1) * eps2;
    float z = sqrtf(1.0f - eps2 * eps2);
    direction =
        vector_plus(
            vector_plus(
                vector_multiplyf(tangent, x),
                vector_multiplyf(bitangent, y)
            ),
            vector_multiplyf(hit_normal, z)
        );

    direction = vector_normalize(direction);
    start = vector_plus(hit.position, vector_multiplyf(hit_normal, 0.001f));

    Vector this_color = all_colors[hit.index];

    Vector emittance = {};//hit.index == 1 ? (Vector){10.0,10.0,10.0} : (Vector){0,0,0};
    resulting_color = vector_plus(resulting_color, vector_multiply(throughput, emittance));
    throughput = vector_multiply(throughput, this_color);
    if (num_bounces > 2) {  // Let first few bounces always continue
       float p = fmaxf(throughput.x, fmaxf(throughput.y, throughput.z));
       if (make_random() > p) {
           break;  
       }
       throughput = vector_dividef(throughput, p);  
    }
  }
  
  return resulting_color;
}

ObjectHit ray_sphere_intersection(Sphere *spheres, int num_spheres, Vector start, Vector direction) {
    float shortest_distance = 999999;
    bool hit = false;
    int hit_index = -1;

    for(int i=0; i < num_spheres;i++) {
        Vector c = spheres[i].position;
        float radius = spheres[i].radius;
        Vector v = vector_minus(start, c);
        float v_dot_direction = vector_dot(v, direction);

        float wee = v_dot_direction * v_dot_direction -
            (vector_dot(v,v)- radius*radius);

        if(wee <= 0.0) {
            continue;
        }

        float dot_product = v_dot_direction * -1;
        float wee_sqrt = sqrtf(wee);
        float intersection1 = dot_product + wee_sqrt;
        float intersection2 = dot_product - wee_sqrt;

        Vector end_position;
        if (intersection1 < intersection2 && intersection1 > 0.00001) {
            end_position = vector_multiplyf(direction, intersection1);
            hit = true;
        } else if (intersection2 < intersection1 && intersection2 > 0.00001) {
            end_position = vector_multiplyf(direction, intersection2);
            hit = true;
        } else {
            continue;
        }

        float length = vector_length(end_position);
        if(hit==true && length < shortest_distance) {
            hit_index = i;
            shortest_distance = length;
        }
    }

    if(hit) {
        ObjectHit res = {.position = vector_plus(start,vector_multiplyf(direction,shortest_distance)), .index = hit_index};
        return res;
    } else {
        ObjectHit res = {{}, -1};
        return res;
    }
}

