#include "asteroids.h"
#include "include/raylib.h"
#include <assert.h>

/*
void update_asteroid_veritices(Asteroid *a) {
    for (int i = 0; i < a->vertices.size; i++) {
        PolarCoords v = a->vertices.coords[i];
        Vector2 center = a->center;

        a->vertices.coords[i] = polar_to_vector(v, a->center, a->angle);
    }
}
*/

Asteroid* init_asteroid(float x, float y, float direction) {
    int coords_size = 7;
    
    Asteroid* a = malloc(sizeof(Asteroid));
    assert(a != NULL && "Can't allocate asteroid");

    a->coords = malloc(sizeof(PolarCoords) * coords_size);
    assert(a->coords != NULL && "Can't allocate coords");

    a->vector_coords = malloc(sizeof(Vector2) * coords_size);
    assert(a->vector_coords != NULL && "Can't allocate vector_coords");

    a->center = (Vector2) {x, y};
    a->rotation_speed = (float)GetRandomValue(1, 10) / 100;
    a->move_speed = (float)GetRandomValue(1, 10) / 10;
    a->direction = direction;
    a->angle = 0.0;
    a->max_radius = 50;
    a->coords_size = coords_size;

    // a->coords[7] = (PolarCoords){50, 0};
    a->coords[6] = (PolarCoords){50, PI / 6};
    a->coords[5] = (PolarCoords){30, PI / 3};
    a->coords[4] = (PolarCoords){20, PI / 2};
    a->coords[3] = (PolarCoords){50, (3 * PI) / 4};
    a->coords[2] = (PolarCoords){30, (4 * PI) / 3};
    a->coords[1] = (PolarCoords){45, (7 * PI) / 4};
    a->coords[0] = (PolarCoords){50, 0};

    for (int i = 0; i < coords_size; i++) {
	a->vector_coords[i] = polar_to_vector(a->coords[i], a->center, a->angle);
    }

    return a;
}

void move_asteroid(Asteroid *a) {
    assert(a != NULL && "No asteroid to move");    
    float angle = fmod(a->angle - a->rotation_speed, (2 * PI));
    a->angle = angle;

    for (int i = 0; i < a->coords_size; i++) {
	a->center.x += cos(a->direction) * a->move_speed;
	a->center.y -= sin(a->direction) * a->move_speed;

	a->vector_coords[i] = polar_to_vector(a->coords[i], a->center, a->angle);
    }
}

void free_asteroid(Asteroid *a) {
    free(a->vector_coords);
    free(a->coords);
    free(a);
}

AsteroidsVector make_asteroids_vector(int size) {
    AsteroidsVector v = malloc(sizeof(Asteroid) * size + sizeof(int) * 2);
    assert(v != NULL && "Can't allocate asteroids vector");
    
    *(int*)v = 0; // len
    v = (AsteroidsVector)((int*)v + 1);

    *(int*)v = size; // cap
    v = (AsteroidsVector)((int*)v + 1);

    return v;
}

int asteroids_vector_len(AsteroidsVector v) {
    return *((int *)v - 2);
}

int asteroids_vector_cap(AsteroidsVector v) {
    return *((int *)v - 1);
}

void append_to_asteroids_vector(AsteroidsVector v, Asteroid *a) {
    v[asteroids_vector_len(v)] = a;
    *((int*)v - 2) += 1;
}

void delete_from_asteroids_vector(AsteroidsVector v, int idx) {
    Asteroid* a = v[idx];
    free_asteroid(a);
    
    int bytes_to_copy = (asteroids_vector_len(v) - idx - 1) * sizeof(Asteroid*);

    AsteroidsVector ptr1 = v + idx;
    AsteroidsVector ptr2 = v + idx + 1;
    memmove(ptr1, ptr2, bytes_to_copy);

    *((int*)v - 2) -= 1;
}

void free_asteroid_vector(AsteroidsVector v) {
    v = (AsteroidsVector)((int*)v - 2);
    free(v);
}

void draw_asteroid(Asteroid *a) {
    assert(a != NULL && "No asteroid to draw");
    for (int i = 0; i < a->coords_size; i++) {
	int nextIndex = (i + 1) % a->coords_size;
	DrawLineV(a->vector_coords[i], a->vector_coords[nextIndex], WHITE);
    }
}
