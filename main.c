#include "deque.h"
#include "include/raylib.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ASTEROIDS 9

typedef enum { MOVE_RIGHT, MOVE_UP, MOVE_LEFT, MOVE_DOWN } Direction;
typedef enum { RIGHT, TOP, LEFT, BOTTOM } Side;
typedef enum { RUNNING, GAME_OVER} GameState;

typedef struct {
    float radius;
    float angle;
} PolarCoords;

typedef struct {
    const int width;
    const int height;
} Screen;

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
    int vertices_len;
    PolarCoords vertices[8];
    Vector2 vectors[8];
    float direction;
    float angle;
    float max_radius;
    float rotation_speed;
    float move_speed;
} Asteroid;

void update_ship_vertices(Ship *ship) {
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

void update_asteroid_veritices(Asteroid *a) {
    for (int i = 0; i < a->vertices_len; i++) {
	a->vectors[i] = polar_to_vector(a->vertices[i], a->center, a->angle);
    }
}

Asteroid *init_asteroid(float x, float y, float direction) {
    Asteroid *a = malloc(sizeof(Asteroid));
    assert(a != NULL && "Can't allocate asteroid");

    Vector2 center = {x, y};

    a->center = center;
    a->vertices_len = 8;
    a->angle = 0.0;
    a->direction = direction;
    a->max_radius = 50;
    a->rotation_speed = (float)GetRandomValue(1, 10) / 100;
    a->move_speed = (float)GetRandomValue(1, 10) / 10;

    a->vertices[7] = (PolarCoords){30, 0};
    a->vertices[6] = (PolarCoords){50, PI / 6};
    a->vertices[5] = (PolarCoords){30, PI / 3};
    a->vertices[4] = (PolarCoords){30, PI / 2};
    a->vertices[3] = (PolarCoords){50, (3 * PI) / 4};
    a->vertices[2] = (PolarCoords){30, (4 * PI) / 3};
    a->vertices[1] = (PolarCoords){45, (7 * PI) / 4};
    a->vertices[0] = (PolarCoords){30, 0};

    update_asteroid_veritices(a);

    return a;
}

void move_asteroid(Asteroid *a) {
    float angle = fmod(a->angle - a->rotation_speed, (2 * PI));
    a->angle = angle;

    for (int i = 0; i < a->vertices_len; i++) {
	a->center.x = a->center.x + cos(a->direction) * a->move_speed;
	a->center.y = a->center.y - sin(a->direction) * a->move_speed;

	a->vectors[i] = polar_to_vector(a->vertices[i], a->center, a->angle);
    }
}

void move_ship(Ship *ship, float speed, Direction dir, Screen screen) {
    switch (dir) {
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

    if (ship->center.x + ship->max_radius < 0) {
	ship->center.x = screen.width + ship->max_radius - 1;
    }

    if (ship->center.x - ship->max_radius > screen.width) {
	ship->center.x = 0 - ship->max_radius + 1;
    }

    if (ship->center.y + ship->max_radius < 0) {
	ship->center.y = screen.height + ship->max_radius - 1;
    }

    if (ship->center.y - ship->max_radius > screen.height) {
	ship->center.y = 0 - ship->max_radius;
    }

    update_ship_vertices(ship);
}

Projectile *init_projectile(Vector2 center, float direction) {
    Projectile *p = malloc(sizeof(Projectile));
    assert(p != NULL && "Can't allocate projectile");

    p->center = center;
    p->direction = direction;
    p->radius = 5;

  return p;
}

void fire(Ship *ship, Deque *projectiles_dq) {
    Projectile *p = init_projectile(ship->vertices[0], ship->direction);
    prepend_node(projectiles_dq, p);
}

bool is_visible(Vector2 point, float radius, Screen screen) {
    return point.x + radius >= 0 && point.x - radius <= screen.width &&
	point.y + radius >= 0 && point.y - radius <= screen.height;
}

void move_projectile_forward(Projectile *p, float speed) {
    p->center.x = p->center.x + cos(p->direction) * speed;
    p->center.y = p->center.y - sin(p->direction) * speed;
}

Node *check_projectile_asteroids_collision(Projectile *p, Node *asteroids) {
    Node *current_node = asteroids;
    while (current_node != NULL) {
	if (current_node->val != NULL) {
	    Asteroid *a = (Asteroid *)current_node->val;

	    if (CheckCollisionCircles(a->center, a->max_radius, p->center, p->radius)) {
		if (CheckCollisionPointPoly(p->center, a->vectors, a->vertices_len)) {
		    return current_node;
		}

		if (CheckCollisionPointPoly((Vector2){p->center.x + p->radius, p->center.y}, a->vectors, a->vertices_len)) {
		    return current_node;
		}

		if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y + p->radius}, a->vectors, a->vertices_len)) {
		    return current_node;
		}

		if (CheckCollisionPointPoly((Vector2){p->center.x - p->radius, p->center.y}, a->vectors, a->vertices_len)) {
		    return current_node;
		}

		if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y - p->radius}, a->vectors, a->vertices_len)) {
		    return current_node;
		}
	    }
	}
	current_node = current_node->next;
    }
    return NULL;
}

bool check_ship_asteroid_collision(Ship* ship, Asteroid* a) {
    if (CheckCollisionCircles(ship->center, ship->max_radius, a->center, a->max_radius)) {
	if (CheckCollisionPointTriangle(a->center, ship->vertices[0], ship->vertices[1], ship->vertices[2])) {
	    return true;
	}

	for (int i = 0; i < a->vertices_len; i++) {
	    if (CheckCollisionPointTriangle(a->vectors[i], ship->vertices[0], ship->vertices[1], ship->vertices[2])) {
		return true;
	    }
	}

	for (int i = 0; i < 3; i++) {
	    if (CheckCollisionPointPoly(ship->vertices[i], a->vectors, a->vertices_len)) {
		return true;
	    }
	}
    }
    
    return false;
}

void draw_projectiles(Deque* projectiles_dq) {
    Node* curr_node = projectiles_dq->first;
    while (curr_node->next) {
	if (curr_node->val != NULL) {
	    Projectile *p = (Projectile *)curr_node->val;
	    DrawCircleV(p->center, p->radius, RED);
	}
	curr_node = curr_node->next;
    }
}

void draw_asteroids(Deque* asteroids_dq) {
    Node* curr_node = asteroids_dq->first;
    while (curr_node->next) {
	if (curr_node->val != NULL) {
	    Asteroid *a = (Asteroid *)curr_node->val;
	    DrawSplineLinear(a->vectors, a->vertices_len, 1, WHITE);
	}
	curr_node = curr_node->next;
    }
}

void draw_info(int projectiles_count, int asteroids_count, int score) {
    char info_buffer[20];
    sprintf(info_buffer, "Projectiles: %d", projectiles_count);
    DrawText(info_buffer, 10, 10, 35, GREEN);
    
    sprintf(info_buffer, "Asteroids: %d", asteroids_count);
    DrawText(info_buffer, 10, 50, 35, GREEN);
    
    sprintf(info_buffer, "Score: %d", score);
    DrawText(info_buffer, 10, 90, 35, GREEN);
    
    DrawFPS(10, 130);
}

void draw_game_over() {
    char buffer[20];
    sprintf(buffer, "Game Over");
    DrawText(buffer, 500, 600, 35, WHITE);    
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    const Screen screen = {.width = 1800, .height = 1450};
    const float rotation_speed = 0.06;
    const float move_speed = 3;
    const float projectile_speed = 9;
    int score = 0;
    GameState game_state = RUNNING;

    Ship ship = init_ship(500, 500);

    Deque *projectiles_dq = init_deque();
    Deque *asteroids_dq = init_deque();

    InitWindow(screen.width, screen.height, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
	// Update
	//----------------------------------------------------------------------------------
	switch (game_state) {
	case RUNNING:
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
                fire(&ship, projectiles_dq);
	    }

	    Node *projectile_node = projectiles_dq->first;
	    while (projectile_node != NULL) {
                if (projectile_node->val != NULL) {
                    Projectile *p = (Projectile *)projectile_node->val;
                    if (is_visible(p->center, p->radius, screen)) {
			Node *collision_node = NULL;
			if ((collision_node =
			     check_projectile_asteroids_collision(
				 p, asteroids_dq->first)) != NULL) {
			    Node *curr_ptr = projectile_node;

			    projectile_node = projectile_node->next;

			    remove_node(projectiles_dq, curr_ptr);

			    remove_node(asteroids_dq, collision_node);

			    score++;
			} else {
			    move_projectile_forward(p, projectile_speed);
			    projectile_node = projectile_node->next;
			}
                    } else {
			Node *curr_ptr = projectile_node;

			projectile_node = projectile_node->next;

			remove_node(projectiles_dq, curr_ptr);
                    }
                } else {
                    projectile_node = projectile_node->next;
                }
	    }

	    Node *asteroid_node = asteroids_dq->first;
	    while (asteroid_node != NULL) {
                if (asteroid_node->val != NULL) {
                    Asteroid *a = (Asteroid *)asteroid_node->val;
		    if (!is_visible(a->center, a->max_radius, screen)) {
			Node *curr_ptr = asteroid_node;

			asteroid_node = asteroid_node->next;

			remove_node(asteroids_dq, curr_ptr);
			continue;
		    }

		    if (check_ship_asteroid_collision(&ship, a)) {
			game_state = GAME_OVER;
			asteroid_node = asteroid_node->next;
			continue;
		    }
		    
		    move_asteroid(a);
		    asteroid_node = asteroid_node->next;
                } else {
                    asteroid_node = asteroid_node->next;
                }
	    }

	    if (MAX_ASTEROIDS > asteroids_dq->count && (GetRandomValue(0, 60) == 4)) {
                Asteroid *a;
                float direction;

                switch (GetRandomValue(RIGHT, BOTTOM)) {
                case RIGHT:
                    direction = GetRandomValue(90, 270) * DEG2RAD;
                    a = init_asteroid(screen.width,
                                      GetRandomValue(0, screen.height),
                                      direction);
                    break;
                case TOP:
                    direction = GetRandomValue(180, 360) * DEG2RAD;
                    a = init_asteroid(GetRandomValue(180, 360), 0, direction);
                    break;
                case LEFT:
                    direction = (GetRandomValue(270, 450) % 360) * DEG2RAD;
                    a = init_asteroid(0, GetRandomValue(0, screen.height),
                                      direction);
                    break;
                case BOTTOM:
                    direction = (GetRandomValue(270, 450) % 360) * DEG2RAD;
                    a = init_asteroid(GetRandomValue(0, 180), screen.height,
                                      direction);
                    break;
                }

                prepend_node(asteroids_dq, a);
	    }

	    break;
	case GAME_OVER:
	    
	    break;
	}

        //----------------------------------------------------------------------------------

	// Draw
	//----------------------------------------------------------------------------------
	BeginDrawing();

	ClearBackground(DARKGRAY);

	switch(game_state){
	case RUNNING:
	    DrawTriangleLines(ship.vertices[0], ship.vertices[1], ship.vertices[2], WHITE);
	    
	    draw_projectiles(projectiles_dq);
	    
	    draw_asteroids(asteroids_dq);
	    
	    draw_info(projectiles_dq->count, asteroids_dq->count, score);
	    break;
	case GAME_OVER:
            draw_game_over();
	    break;
	}
	
	EndDrawing();
	//----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
