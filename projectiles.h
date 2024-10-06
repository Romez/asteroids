#include "include/raylib.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
    Vector2 center;
    float direction;
    float radius;
} Projectile;

typedef Projectile **ProjectilesVector;

Projectile *make_projectile(Vector2 center, float direction);

ProjectilesVector make_projectiles_vector(int size);

int projectiles_vector_len(ProjectilesVector v);

int projectiles_vector_cap(ProjectilesVector v);

void free_projectile(Projectile *p);

void expand_projectiles_vector(ProjectilesVector* old_v, int size);

void append_to_projectiles_vector(ProjectilesVector *v, Projectile *p);

void delete_from_projectiles_vector(ProjectilesVector v, int idx);
