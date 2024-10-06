#include <math.h>
#include "include/raylib.h"

typedef struct {
    float radius;
    float angle;
} PolarCoords;

Vector2 polar_to_vector(PolarCoords pc, Vector2 center, float angle);
