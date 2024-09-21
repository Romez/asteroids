#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "include/raylib.h"

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
} Ship;

typedef struct {
    Vector2 center;
    float direction;
    float radius;
} Projectile;

typedef enum { MOVE_RIGHT, MOVE_UP, MOVE_LEFT, MOVE_DOWN } Direction;

typedef struct {
    Vector2 center;
    Vector2 vertices[8];
    float direction;
    float angle;
} Asteroid;

void update_ship_vertices(Ship* ship) {
    int k = 30;
    float l = (3 * PI) / 4;
    float m = (5 * PI) / 4;

    ship->vertices[0].x = ship->center.x + cos(ship->direction) * k;
    ship->vertices[0].y = ship->center.y - sin(ship->direction) * k;

    ship->vertices[1].x = ship->center.x + cos(ship->direction + l) * k;
    ship->vertices[1].y = ship->center.y - sin(ship->direction + l) * k;

    ship->vertices[2].x = ship->center.x + cos(ship->direction + m) * k;
    ship->vertices[2].y = ship->center.y - sin(ship->direction + m) * k;
}

Ship init_ship(float x, float y) {
    Ship ship = {
	.center = (Vector2){x, y},
	.direction = 0.0,
    };
    update_ship_vertices(&ship);

    return ship;
}

Asteroid* init_asteroid(float x, float y) {
    Asteroid* a = malloc(sizeof(Asteroid));
    assert(a != NULL && "Can't allocate asteroid");

    Vector2 center = {x, y};

    a->center = center;
    a->angle = 0.0;
    a->direction = 0.0;

    a->vertices[0] = (Vector2) {center.x + 30, center.y};
    a->vertices[1] = (Vector2) {center.x + 25, center.y - 25};
    a->vertices[2] = (Vector2) {center.x + 5, center.y - 35};
    a->vertices[3] = (Vector2) {center.x - 20, center.y - 25};
    a->vertices[4] = (Vector2) {center.x - 30, center.y - 5};
    a->vertices[5] = (Vector2) {center.x - 10, center.y + 20};
    a->vertices[6] = (Vector2) {center.x + 10, center.y + 15};
    a->vertices[7] = (Vector2) {center.x + 30, center.y};
    return a;
}

/*
void *rotate_asteroid(Asteroid* asteroid, float rotate_speed) {
    int k = 30;
    float l = (3 * PI) / 4;
    float m = (5 * PI) / 4;

    asteroid->angle = fmod(asteroid->angle - rotate_speed, (2 * PI));
    for(int i = 0; i < 8; i++) {
        Vector2 v = asteroid->vertices[i];
        asteroid->vertices[i].x =


        ship->vertices[0].x = ship->center.x + cos(ship->direction) * k;
    ship->vertices[0].y = ship->center.y - sin(ship->direction) * k;

    }
}
*/

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

void prepend_node(Node* dq, void* val) {
    Node* n = malloc(sizeof(Node));
    assert(n != NULL && "Can't allocate sentinel first");

    n->next =  dq->next;
    n->prev = dq;
    n->val = val;

    Node* t = dq->next;

    dq->next = n;
    t->prev = n;
}

void fire(Ship* ship, Node* projectiles_dq, int* projectiles_count) {
    Projectile* p = init_projectile(ship->vertices[0], ship->direction);

    prepend_node(projectiles_dq, p);

    *projectiles_count += 1;
}

bool is_visible(Vector2 point, Screen screen) {
    return point.x < 0 || point.x > screen.width || point.y < 0 || point.y > screen.height;
}

void move_projectile_forward(Projectile* p, float speed) {
    p->center.x = p->center.x + cos(p->direction) * speed;
    p->center.y = p->center.y - sin(p->direction) * speed;
}

Node* collision_with(Projectile* p, Node* asteroids) {
    Node* current_node = asteroids;
    while(current_node != NULL) {
	if (current_node->val != NULL) {
	    Asteroid* a = (Asteroid*) current_node->val;

	    if (CheckCollisionPointPoly(p->center, a->vertices, 8)) {
		return current_node;
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

void remove_node(Node* node) {
    free(node->val);
    Node* prev_node = node->prev;
    Node* next_node = node->next;

    prev_node->next = next_node;
    next_node->prev = prev_node;

    free(node);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const Screen screen = { .width = 1800, .height = 1450};
    const float rotation_speed = 0.2;
    const float move_speed = 5;
    const float projectile_speed = 7;
    int projectiles_count = 0;

    Ship ship = init_ship(500, 500);

    Node* projectiles_dq = init_deque();
    Node* asteroids_dq = init_deque();

    Asteroid* asteroid = init_asteroid(900, 500);
    prepend_node(asteroids_dq, asteroid);

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
		Projectile* p = (Projectile*) curr_node->val;

		if (is_visible(p->center, screen) || collision_with(p, asteroids_dq) != NULL) {
		    Node* curr_ptr = curr_node;

		    curr_node = curr_node->next;

		    projectiles_count -= 1;

		    remove_node(curr_ptr);
		} else {
		    move_projectile_forward(p, projectile_speed);
		    curr_node = curr_node->next;
		}
	    } else {
		curr_node = curr_node->next;
	    }
	}

	curr_node = asteroids_dq;
	while (curr_node != NULL)
	{
	    if (curr_node->val != NULL) {
		Asteroid* a = (Asteroid*) curr_node->val;

		// if (is_visible(a->center, screen)) {
		//     Node* curr_ptr = curr_node;

		//     curr_node = curr_node->next;

		//     projectiles_count -= 1;

		//     remove_node(curr_ptr);
		// } else {
		//move_projectile_forward(a, projectile_speed);
		curr_node = curr_node->next;
		    //}
	    } else {
		curr_node = curr_node->next;
	    }
	}

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

	ClearBackground(DARKGRAY);

	DrawCircleV(ship.center, 5, WHITE);

	DrawCircleV(ship.vertices[0], 5, RED);
	DrawCircleV(ship.vertices[1], 5, GREEN);
	DrawCircleV(ship.vertices[2], 5, BLUE);

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
		Vector2 points[9];
		points[0] = a->center;
		for (int i = 0; i < 9; i++) {
		    points[i+1] = a->vertices[i];
		}

		DrawTriangleFan(points, 9, WHITE);
	    }
	    curr_node = curr_node->next;
	}

	char projectiles_buffer[20];
	sprintf(projectiles_buffer, "Projectiles: %d", projectiles_count);
	DrawText(projectiles_buffer, 10, 10, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
