#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600
// For the file
unsigned char image_data[IMAGE_WIDTH * IMAGE_HEIGHT * 3];


float picture[IMAGE_WIDTH][IMAGE_HEIGHT][3];

const int numRays = 10; // Rays per iteration

Sphere all_spheres[] = {{.position.x=-1.5, .position.z=-4, .radius=1}, {.position.x=1.5,.position.z=-4, .radius=1}};

float makeRandom(void) { return (float)rand() / (float)RAND_MAX; }

float clamp(float x, float min, float max) { 
  return (x < min ? min : (x > max ? max : x));
}

Vector shoot_ray(Sphere *spheres, int num_spheres, Vector start, Vector direction);
ObjectHit ray_sphere_intersection(Sphere *spheres, int num_spheres, Vector start, Vector direction);

int main(void) {
    srand(0);
    Vector s = {0,0,0};
    float xmax = 5, ymax = 5;
    int image_index = 0;
    for(int screenY = 0; screenY < IMAGE_HEIGHT; screenY++) {
        for(int screenX = 0; screenX < IMAGE_WIDTH; screenX++) {
            Vector end_color = {};
            float x, y;
            x = (float)(screenX * 6) / (float)IMAGE_WIDTH - 3.0;
            y = (float)(screenY * 6) * (float)IMAGE_HEIGHT / (float)IMAGE_WIDTH /
                (float)IMAGE_HEIGHT - 3.0 * (float)IMAGE_HEIGHT / (float)IMAGE_WIDTH;

            Vector dir =  {.x = x / xmax, .y = y / ymax, .z = -1};
            dir = vector_normalize(dir);

            for(int i = 0; i < numRays; i++) {
                // bounces = 0
                
                end_color = vector_plus(end_color, shoot_ray(all_spheres, 2, s, dir));
                //printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);
            }
            end_color = vector_dividef(end_color, (float)numRays);
            printf("%f %f %f\n", end_color.x, end_color.y, end_color.z);

            picture[screenX][screenY][0] = end_color.x;
            picture[screenX][screenY][1] = end_color.y;
            picture[screenX][screenY][2] = end_color.z;
        }
    }
    for(int screenY = 0; screenY < IMAGE_HEIGHT; screenY++) {
        for(int screenX = 0; screenX < IMAGE_WIDTH; screenX++) {
            float r = picture[screenX][screenY][0];
            float g = picture[screenX][screenY][1];
            float b = picture[screenX][screenY][2];
            short end_r = clamp(r,0,1) * 255;
            short end_g = clamp(g,0,1) * 255;
            short end_b = clamp(b,0,1) * 255;
            image_data[image_index] = (unsigned char)end_r;
            image_data[image_index+1] = (unsigned char)end_g;
            image_data[image_index+2] = (unsigned char)end_b;
            image_index+=3;
        }
    }

   save_image(image_data, "result.ppm",800,600);

}

Vector shoot_ray(Sphere *spheres, int num_spheres, Vector start, Vector direction) {
  int max_bounces = 50; 
  Vector resulting_color = {};
  Vector throughput = {1.0, 1.0, 1.0};
 /* 
    ObjectHit hit = ray_sphere_intersection(all_spheres, num_spheres, start, direction);
    if (hit.index != -1) {
        return (Vector){1.0,1.0,1.0};
    }
    else return (Vector){};
    */

  for(int num_bounces = 0; num_bounces < max_bounces; num_bounces++) {
    ObjectHit hit = ray_sphere_intersection(all_spheres, num_spheres, start, direction);

    if (hit.index == -1) {
      break;
    }

    Vector new_direction = {.x = 2 * makeRandom() - 1, .y=2 * makeRandom() -1, .z=2*makeRandom() -1};

    Sphere hit_sphere = all_spheres[hit.index];
   
    Vector hit_normal = vector_minus(hit.position, hit_sphere.position);
    hit_normal = vector_normalize(hit_normal);
    new_direction = vector_cross(new_direction, hit_normal);
    new_direction = vector_normalize(new_direction);

    float eps1 = makeRandom() * 6.28318; // 2*PI
    float eps2 = sqrtf(makeRandom());

    float x = cosf(eps1) * eps2;
    float y = sinf(eps1) * eps2;
    float z = sqrtf(1.0f - eps2 * eps2);
    Vector ssx = vector_plus(vector_plus(vector_multiplyf(new_direction, x), vector_multiplyf(vector_cross(hit_normal,new_direction), y)), vector_multiplyf(hit_normal, z));
    ssx = vector_normalize(ssx);
    direction = ssx;
    start = vector_plus(hit.position, vector_multiplyf(hit_normal, 0.001f));

    Vector this_color = {0.3,0.8,0.1};//Vector(all_colors[hit.index].r, all_colors[hit.index].g, all_colors[hit.index].b);

    Vector emittance = hit.index == 1 ? (Vector){10.0,10.0,10.0} : (Vector){0,0,0};
    //resulting_color = vector_plus(emittance, vector_multiply(resulting_color, this_color)); 
    //resulting_color = vector_multiply(resulting_color, this_color); 
    resulting_color = vector_plus(resulting_color, vector_multiply(throughput, emittance));
    throughput = vector_multiply(throughput, this_color);
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
            (v.x*v.x + v.y*v.y + v.z*v.z - radius*radius);

        if(wee <= 0.0) {
            continue;
        }

        float dot_product = v_dot_direction * -1;
        float wee_sqrt = sqrt(wee);
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

