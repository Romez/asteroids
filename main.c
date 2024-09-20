#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "include/raylib.h"

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
} Projectile;

typedef enum {
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_LEFT,
    MOVE_DOWN
} Direction;

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

void move_ship(Ship* ship, float speed, Direction dir) {
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

    return p;
}

void fire(Ship* ship, Node* projectiles_dq, int* projectiles_count) {
    Projectile* p = init_projectile(ship->vertices[0], ship->direction);

    Node* n = malloc(sizeof(Node));
    assert(n != NULL && "Can't allocate sentinel first");

    n->next =  projectiles_dq->next;
    n->prev = projectiles_dq;
    n->val = p;

    Node* t = projectiles_dq->next;

    projectiles_dq->next = n;
    t->prev = n;

    *projectiles_count += 1;
}

void move_projectile_forward(Projectile* p) {
    p->center.x = p->center.x + cos(p->direction) * 3;
    p->center.y = p->center.y - sin(p->direction) * 3;
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
    const int screenWidth = 1800;
    const int screenHeight = 1450;
    const float rotation_speed = 0.2;
    const float move_speed = 5;
    int projectiles_count = 0;

    Ship ship = init_ship(500, 500);

    Node* projectiles_dq = init_deque();

    InitWindow(screenWidth, screenHeight, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
	if (IsKeyDown(KEY_LEFT)) {
	    move_ship(&ship, rotation_speed, MOVE_LEFT);
	}

	if (IsKeyDown(KEY_RIGHT)) {
	    move_ship(&ship, rotation_speed, MOVE_RIGHT);
	}

	if (IsKeyDown(KEY_UP)) {
	    move_ship(&ship, move_speed, MOVE_UP);
	}

	if (IsKeyDown(KEY_DOWN)) {
	    move_ship(&ship, move_speed, MOVE_DOWN);
	}

	if (IsKeyPressed(KEY_SPACE)) {
	    fire(&ship, projectiles_dq, &projectiles_count);
	}

	Node* curr_node = projectiles_dq;
	while (curr_node != NULL)
	{
	    if (curr_node->val != NULL) {
		Projectile* p = (Projectile*) curr_node->val;
		if (p->center.x < 0 || p->center.x > screenWidth || p->center.y < 0 || p->center.y > screenHeight) {
		    Node* curr_ptr = curr_node;

		    curr_node = curr_node->next;

		    projectiles_count -= 1;

		    remove_node(curr_ptr);
		} else {
		    move_projectile_forward(curr_node->val);
		    curr_node = curr_node->next;
		}
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

	Node* pr1 = projectiles_dq;
	while (pr1->next) {
	    if (pr1->val != NULL) {
		Projectile* p1 = (Projectile*) pr1->val;
		DrawCircleV(p1->center, 5, RED);
	    }
	    pr1 = pr1->next;
	}

	char projectiles_buffer[20];
	sprintf(projectiles_buffer, "Projectiles: %d", projectiles_count);
	DrawText(projectiles_buffer, 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
