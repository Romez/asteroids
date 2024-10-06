#include "include/raylib.h"
#include "polar.h"
#include "stdlib.h"
#include <assert.h>
#include <string.h>

typedef struct {
    Vector2 center;
    float rotation_speed;
    float move_speed;    
    float direction;
    float angle;
    float max_radius;    
    int coords_size;
    PolarCoords* coords;
    Vector2* vector_coords;
} Asteroid;

typedef Asteroid** AsteroidsVector;

Asteroid* init_asteroid(float x, float y, float direction);

void move_asteroid(Asteroid *a);

void free_asteroid(Asteroid *a);

AsteroidsVector init_asteroids_vector(int size);

int asteroids_vector_len(AsteroidsVector v);

int asteroids_vector_cap(AsteroidsVector v);

void append_to_asteroids_vector(AsteroidsVector v, Asteroid *a);

void delete_from_asteroids_vector(AsteroidsVector v, int idx);

void free_asteroid_vector(AsteroidsVector a);

void draw_asteroid(Asteroid *a);
