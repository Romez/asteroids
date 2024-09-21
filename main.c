#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "include/raylib.h"

#define MAX_ASTEROIDS 9

typedef enum { MOVE_RIGHT, MOVE_UP, MOVE_LEFT, MOVE_DOWN } Direction;
typedef enum { RIGHT, TOP, LEFT, BOTTOM} Side;

typedef struct {
    float radius;
    float angle;
} PolarCoords;

typedef struct {
    const int width;
    const int height;
} Screen;

typedef struct Node {
    struct Node* prev;
    struct Node* next;
    void* val;
} Node;

typedef struct {
    Vector2 center;
    float direction;
    Vector2 vertices[3];
    float max_radius;
} Ship;

typedef struct {
    Vector2 center;
    float direction;
    float radius;
} Projectile;

typedef struct {
    Vector2 center;
    PolarCoords vertices[8];
    Vector2 vectors[8];
    float direction;
    float angle;
    float max_radius;
} Asteroid;

void update_ship_vertices(Ship* ship) {
    float l = (3 * PI) / 4;
    float m = (5 * PI) / 4;

    ship->vertices[0].x = ship->center.x + cos(ship->direction) * ship->max_radius;
    ship->vertices[0].y = ship->center.y - sin(ship->direction) * ship->max_radius;

    ship->vertices[1].x = ship->center.x + cos(ship->direction + l) * ship->max_radius;
    ship->vertices[1].y = ship->center.y - sin(ship->direction + l) * ship->max_radius;

    ship->vertices[2].x = ship->center.x + cos(ship->direction + m) * ship->max_radius;
    ship->vertices[2].y = ship->center.y - sin(ship->direction + m) * ship->max_radius;
}

Ship init_ship(float x, float y) {
    Ship ship = {
	.center = (Vector2){x, y},
	.direction = 0.0,
	.max_radius = 30,
    };
    update_ship_vertices(&ship);

    return ship;
}

Vector2 polar_to_vector(PolarCoords pc, Vector2 center, float angle) {
    Vector2 v = {
        .x = center.x + pc.radius * cos(pc.angle - angle),
        .y = center.y + pc.radius * sin(pc.angle - angle),
    };
    return v;
}

void update_asteroid_vectors(Asteroid* a) {
    for (int i = 0; i < 8; i++) {
	a->vectors[i] = polar_to_vector(a->vertices[i], a->center, a->angle);
    }
}

Asteroid* init_asteroid(float x, float y, float direction) {
    Asteroid* a = malloc(sizeof(Asteroid));
    assert(a != NULL && "Can't allocate asteroid");

    Vector2 center = {x, y};

    a->center = center;
    a->angle = 0.0;
    a->direction = direction;
    a->max_radius = 50;

    a->vertices[7] = (PolarCoords) {30, 0};
    a->vertices[6] = (PolarCoords) {50, PI / 6};
    a->vertices[5] = (PolarCoords) {30, PI / 3};
    a->vertices[4] = (PolarCoords) {30, PI / 2};
    a->vertices[3] = (PolarCoords) {50, (3 * PI) / 4};
    a->vertices[2] = (PolarCoords) {30, (4 * PI) / 3};
    a->vertices[1] = (PolarCoords) {45, (7 * PI) / 4};
    a->vertices[0] = (PolarCoords) {30, 0};

    update_asteroid_vectors(a);

    return a;
}

void move_asteroid(Asteroid* a, float rotate_speed) {
    float angle = fmod(a->angle - rotate_speed, (2 * PI));
    a->angle = angle;

    for (int i = 0; i < 8; i++) {
	a->center.x = a->center.x + cos(a->direction) * .2;
	a->center.y = a->center.y - sin(a->direction) * .2;

        a->vectors[i] = polar_to_vector(a->vertices[i], a->center, a->angle);
    }
}

void move_ship(Ship* ship, float speed, Direction dir, Screen screen) {
    switch(dir) {
    case MOVE_LEFT:
	ship->direction = fmod(ship->direction + speed, (2 * PI));
	break;
    case MOVE_RIGHT:
	ship->direction = fmod(ship->direction - speed, (2 * PI));
	break;
    case MOVE_UP:
	ship->center.x = ship->center.x + cos(ship->direction) * speed;
	ship->center.y = ship->center.y - sin(ship->direction) * speed;
	break;
    case MOVE_DOWN:
	ship->center.x = ship->center.x - cos(ship->direction) * speed;
	ship->center.y = ship->center.y + sin(ship->direction) * speed;
	break;
    }
    update_ship_vertices(ship);
}

Projectile* init_projectile(Vector2 center, float direction) {
    Projectile* p = malloc(sizeof(Projectile));
    assert(p != NULL && "Can't allocate projectile");

    p->center = center;
    p->direction = direction;
    p->radius = 5;

    return p;
}

void prepend_node(Node* dq, void* val, int* count) {
    Node* n = malloc(sizeof(Node));
    assert(n != NULL && "Can't allocate sentinel first");

    n->next =  dq->next;
    n->prev = dq;
    n->val = val;

    Node* t = dq->next;

    dq->next = n;
    t->prev = n;

    (*count)++;
}

void fire(Ship* ship, Node* projectiles_dq, int* projectiles_count) {
    Projectile* p = init_projectile(ship->vertices[0], ship->direction);

    prepend_node(projectiles_dq, p, projectiles_count);
}

bool is_visible(Vector2 point, float radius, Screen screen) {
    return point.x + radius >= 0 && point.x - radius <= screen.width && point.y + radius >= 0 && point.y - radius <= screen.height;
}

void move_projectile_forward(Projectile* p, float speed) {
    p->center.x = p->center.x + cos(p->direction) * speed;
    p->center.y = p->center.y - sin(p->direction) * speed;
}

Node* collision_with(Projectile* p, Node* asteroids) {
    Node *current_node = asteroids;
    while(current_node != NULL) {
	if (current_node->val != NULL) {
          Asteroid *a = (Asteroid *)current_node->val;

          if (CheckCollisionCircles(a->center,a-> max_radius, p->center, p->radius)) {
	      if (CheckCollisionPointPoly(p->center, a->vectors, 8)) {
		  return current_node;
              }

	      if (CheckCollisionPointPoly((Vector2){p->center.x + p->radius, p->center.y}, a->vectors, 8)) {
		  return current_node;
              }

	      if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y + p->radius}, a->vectors, 8)) {
		  return current_node;
              }

	      if (CheckCollisionPointPoly((Vector2){p->center.x - p->radius, p->center.y}, a->vectors, 8)) {
		  return current_node;
              }

	      if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y - p->radius}, a->vectors, 8)) {
		  return current_node;
              }
          }
	}
	current_node = current_node->next;
    }
    return NULL;
}

Node* init_deque() {
    Node* sentinel_first = malloc(sizeof(Node));
    assert(sentinel_first != NULL && "Can't allocate sentinel first");

    Node* sentinel_last = malloc(sizeof(Node));
    assert(sentinel_last != NULL && "Can't allocate sentinel last");

    sentinel_first->next = sentinel_last;
    sentinel_first->val = NULL;

    sentinel_last->prev = sentinel_first;
    sentinel_last->val = NULL;

    return sentinel_first;
}

void remove_node(Node* node, int* count) {
    free(node->val);
    Node* prev_node = node->prev;
    Node* next_node = node->next;

    prev_node->next = next_node;
    next_node->prev = prev_node;

    free(node);

    (*count)--;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const Screen screen = { .width = 1800, .height = 1450};
    const float rotation_speed = 0.06;
    const float move_speed = 5;
    const float projectile_speed = 9;
    int projectiles_count = 0;
    int asteroids_count = 0;

    Ship ship = init_ship(500, 500);

    Node* projectiles_dq = init_deque();
    Node *asteroids_dq = init_deque();

    InitWindow(screen.width, screen.height, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
	if (IsKeyDown(KEY_LEFT)) {
	    move_ship(&ship, rotation_speed, MOVE_LEFT, screen);
	}

	if (IsKeyDown(KEY_RIGHT)) {
	    move_ship(&ship, rotation_speed, MOVE_RIGHT, screen);
	}

	if (IsKeyDown(KEY_UP)) {
	    move_ship(&ship, move_speed, MOVE_UP, screen);
	}

	if (IsKeyDown(KEY_DOWN)) {
	    move_ship(&ship, move_speed, MOVE_DOWN, screen);
	}

	if (IsKeyPressed(KEY_SPACE)) {
	    fire(&ship, projectiles_dq, &projectiles_count);
	}

	Node* curr_node = projectiles_dq;
	while (curr_node != NULL)
	{
	    if (curr_node->val != NULL) {
                Projectile *p = (Projectile *)curr_node->val;
                if (is_visible(p->center, p->radius, screen)) {
		    Node *collision_node = NULL;
                    if ((collision_node = collision_with(p, asteroids_dq)) != NULL) {
			Node *curr_ptr = curr_node;

			curr_node = curr_node->next;

			remove_node(curr_ptr, &projectiles_count);

			remove_node(collision_node, &asteroids_count);
                    } else {
			move_projectile_forward(p, projectile_speed);
			curr_node = curr_node->next;
		    }
                } else {
		    Node *curr_ptr = curr_node;

		    curr_node = curr_node->next;

		    remove_node(curr_ptr, &projectiles_count);
		}
	    } else {
		curr_node = curr_node->next;
	    }
	}

	curr_node = asteroids_dq;
        while (curr_node != NULL)
        {
            if (curr_node->val != NULL) {
                Asteroid *a = (Asteroid *)curr_node->val;
                if (is_visible(a->center, a->max_radius, screen)) {
                    move_asteroid(a, 0.03);

                    curr_node = curr_node->next;
                } else {
                    Node* curr_ptr = curr_node;

                    curr_node = curr_node->next;

                    remove_node(curr_ptr, &asteroids_count);
                }
            } else {
                curr_node = curr_node->next;
            }
        }

        if (MAX_ASTEROIDS > asteroids_count && (GetRandomValue(0, 60) == 4)) {
            Asteroid *a;
            float direction;

            switch (GetRandomValue(RIGHT, BOTTOM)) {
            case RIGHT:
		direction = GetRandomValue(90, 270) * DEG2RAD;
		a = init_asteroid(screen.width, GetRandomValue(0, screen.height), direction);
		break;
            case TOP:
		direction = GetRandomValue(180, 360) * DEG2RAD;
		a = init_asteroid(GetRandomValue(180, 360), 0, direction);
		break;
            case LEFT:
		direction = (GetRandomValue(270, 450) % 360) * DEG2RAD;
		a = init_asteroid(0, GetRandomValue(0, screen.height), direction);
		break;
            case BOTTOM:
		direction = (GetRandomValue(270, 450) % 360) * DEG2RAD;
		a = init_asteroid(GetRandomValue(0, 180), screen.height, direction);
		break;
	    }

            prepend_node(asteroids_dq, a, &asteroids_count);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

	ClearBackground(DARKGRAY);

	DrawTriangleLines(ship.vertices[0], ship.vertices[1], ship.vertices[2], WHITE);

	curr_node = projectiles_dq;
	while (curr_node->next)
	{
	    if (curr_node->val != NULL) {
		Projectile* p = (Projectile*) curr_node->val;
		DrawCircleV(p->center, p->radius, RED);
	    }
	    curr_node = curr_node->next;
	}

	curr_node = asteroids_dq;
	while (curr_node->next)
	{
	    if (curr_node->val != NULL) {
		Asteroid* a = (Asteroid*) curr_node->val;
		DrawSplineLinear(a->vectors, 8, 1, WHITE);
	    }
	    curr_node = curr_node->next;
	}

	char projectiles_buffer[20];
        sprintf(projectiles_buffer, "Projectiles: %d", projectiles_count);
	DrawText(projectiles_buffer, 10, 10, 20, DARKGREEN);

        char asteroids_buffer[20];
	sprintf(asteroids_buffer, "Asteroids: %d", asteroids_count);
	DrawText(asteroids_buffer, 10, 30, 20, DARKGREEN);

	DrawFPS(10, 50);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
