#include "polar.h"

Vector2 polar_to_vector(PolarCoords pc, Vector2 center, float angle) {
    center.x += + pc.radius * cos(pc.angle + angle);
    center.y -= pc.radius * sin(pc.angle + angle);
    return center;
}
