#include "projectiles.h"
#include <stdio.h>

Projectile *make_projectile(Vector2 center, float direction) {
    Projectile *p = malloc(sizeof(Projectile));
    assert(p != NULL && "Can't allocate projectile");

    p->center = center;
    p->direction = direction;
    p->radius = 5;

  return p;
}

ProjectilesVector make_projectiles_vector(int size) {
    ProjectilesVector v = malloc(sizeof(Projectile*) * size + sizeof(int) * 2);
    assert(v != NULL && "Can't allocate projectile vector");

    *(int*)v = 0; // len
    v = (ProjectilesVector)((int*)v + 1);

    *(int*)v = size; // cap
    v = (ProjectilesVector)((int*)v + 1);

    return v;
}

int projectiles_vector_len(ProjectilesVector v) {
    return *((int *)v - 2);
}

int projectiles_vector_cap(ProjectilesVector v) {
    return *((int *)v - 1);
}

void free_projectile(Projectile *p) { free(p); }

void free_projectiles_vector(ProjectilesVector v) {
    v = (ProjectilesVector)((int*)v - 2);
    free(v);
}

void expand_projectiles_vector(ProjectilesVector* old_v, int size) {
    int new_size = projectiles_vector_cap(*old_v) + size;
    ProjectilesVector new_v = make_projectiles_vector(new_size);

    memmove(new_v, *old_v, projectiles_vector_len(*old_v) * sizeof(Projectile*));

    *((int *)new_v - 2) = projectiles_vector_len(*old_v);

    free_projectiles_vector(*old_v);
    *old_v = new_v;
}

void append_to_projectiles_vector(ProjectilesVector* v, Projectile* p) {
    if (projectiles_vector_len(*v) == projectiles_vector_cap(*v)) {	
	expand_projectiles_vector(v, projectiles_vector_len(*v));
    }

    (*v)[projectiles_vector_len(*v)] = p;
    *((int *)*v - 2) += 1;
}

void delete_from_projectiles_vector(ProjectilesVector v, int idx) {
    Projectile* p = v[idx];
    free_projectile(p);
    
    int bytes_to_copy = (projectiles_vector_len(v) - idx - 1) * sizeof(Projectile*);

    ProjectilesVector ptr1 = v + idx;
    ProjectilesVector ptr2 = v + idx + 1;
    memmove(ptr1, ptr2, bytes_to_copy);

    *((int*)v - 2) -= 1;
}
