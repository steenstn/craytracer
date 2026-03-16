#include <stdbool.h>
#include <math.h>

typedef struct ObjectHit {
    Vector position;
    int index;
} ObjectHit;

typedef struct Sphere {
    Vector position;
    float radius;
} Sphere;


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

Vector shoot_ray(Sphere *spheres, Vector* colors, int num_spheres, Vector start, Vector direction) {
  int max_bounces = 4; 
  Vector env = {1, 1, 1}; 
  Vector resulting_color = {};
  Vector throughput = {1.0, 1.0, 1.0};

  for(int num_bounces = 0; num_bounces < max_bounces; num_bounces++) {
        ObjectHit hit = ray_sphere_intersection(spheres, num_spheres, start, direction);

    if (hit.index == -1) {
        resulting_color = vector_plus(resulting_color, vector_multiply(throughput, env));
        break;
    }

    Sphere hit_sphere = spheres[hit.index];
   
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

    Vector this_color = colors[hit.index];

    Vector emittance = {};//hit.index%10 == 0 ? (Vector){10.0,10.0,10.0} : (Vector){0,0,0};
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

