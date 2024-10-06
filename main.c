#include "include/raylib.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asteroids.h"

#define MAX_ASTEROIDS 9

typedef enum { MOVE_RIGHT, MOVE_UP, MOVE_LEFT, MOVE_DOWN } Direction;
typedef enum { RIGHT, TOP, LEFT, BOTTOM } Side;
typedef enum { GAME, GAME_OVER, WINNERS} GameScreen;

typedef struct {
    const int width;
    const int height;
} Screen;

typedef struct {
    Vector2 center;
    Vector2 vertices[3];
    float direction;
    float max_radius;
} Ship;

typedef struct {
    Vector2 center;
    float direction;
    float radius;
} Projectile;

typedef struct {
    int score;
    GameScreen game_screen;
} Game;

const Screen screen = {.width = 1800, .height = 1450};
const float rotation_speed = 0.06;
const float move_speed = 5;
const float projectile_speed = 9;

Ship init_ship(Vector2 center);

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

Ship init_ship(Vector2 center) {
    Ship ship = {
	.center = center,
	.direction = 0.0,
	.max_radius = 30,
    };
    update_ship_vertices(&ship);

    return ship;
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

/*
void fire(Ship *ship, Deque *projectiles_dq) {
    Projectile *p = init_projectile(ship->vertices[0], ship->direction);
    prepend_node(projectiles_dq, p);
}
*/

bool is_on_screen(Vector2 point, float radius, Screen screen) {
    return point.x + radius >= 0 && point.x - radius <= screen.width &&
	point.y + radius >= 0 && point.y - radius <= screen.height;
}

void move_projectile_forward(Projectile *p, float speed) {
    p->center.x = p->center.x + cos(p->direction) * speed;
    p->center.y = p->center.y - sin(p->direction) * speed;
}

/*
int check_projectile_asteroids_collision(Projectile *p, Asteroid *asteroids, int
asteroids_len) { for (int i = 0; i < asteroids_len; i++) { Asteroid a =
asteroids[i]; if (CheckCollisionCircles(a.center, a.max_radius, p->center,
p->radius)) { if (CheckCollisionPointPoly(p->center, a.vectors, a.vertices_len))
{ return i;
                }

                if (CheckCollisionPointPoly((Vector2){p->center.x + p->radius,
p->center.y}, a.vectors, a.vertices_len)) { return i;
                }

                if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y +
p->radius}, a.vectors, a.vertices_len)) { return i;
                }

                if (CheckCollisionPointPoly((Vector2){p->center.x - p->radius,
p->center.y}, a.vectors, a.vertices_len)) { return i;
                }

                if (CheckCollisionPointPoly((Vector2){p->center.x, p->center.y -
p->radius}, a.vectors, a.vertices_len)) { return i;
                }
            }
    }

    return -1;
}
*/

bool check_ship_asteroid_collision(Ship* ship, Asteroid* a) {
    if (CheckCollisionCircles(ship->center, ship->max_radius, a->center, a->max_radius)) {
	if (CheckCollisionPointTriangle(a->center, ship->vertices[0], ship->vertices[1], ship->vertices[2])) {
	    return true;
        }

	for (int i = 0; i < a->coords_size; i++) {
	    Vector2 v = a->vector_coords[i];
            if (CheckCollisionPointTriangle(v, ship->vertices[0], ship->vertices[1], ship->vertices[2])) {
		return true;
            }
        }

        for (int i = 0; i < 3; i++) {
            if (CheckCollisionPointPoly(ship->vertices[i], a->vector_coords, a->coords_size)) {
		return true;
            }
        }
    }

    return false;
}

bool check_two_asteroids_collision(Asteroid *a1, Asteroid* a2) {
    if (CheckCollisionCircles(a1->center, a1->max_radius, a2->center, a2->max_radius)) {
	for (int i = 0; i < a1->coords_size; i++) {
	    Vector2 v = a1->vector_coords[i];
	    if (CheckCollisionPointPoly(v, a2->vector_coords, a2->coords_size)) {		
		return true;
	    }
	}

	for (int i = 0; i < a2->coords_size; i++) {
	    Vector2 v = a2->vector_coords[i];
	    if (CheckCollisionPointPoly(v, a1->vector_coords, a1->coords_size)) {
		return true;
	    }
	}
    }

    return false;
}

void draw_ship(Ship ship) {
    DrawTriangleLines(ship.vertices[0], ship.vertices[1], ship.vertices[2], WHITE);
}

/*
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
*/

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

void draw_game_over(Screen screen, char* player) {
    char* game_over = "Game Over";
    int game_over_font_size = 34;
    int game_over_len = MeasureText(game_over, game_over_font_size);
    int game_over_x = screen.width / 2 - game_over_len / 2;
    int game_over_y = screen.height / 2 - (game_over_font_size / 2);
    DrawText(game_over, game_over_x, game_over_y, game_over_font_size, WHITE);

    int rect_width = 400;
    int rect_height = 60;
    int rect_x = screen.width / 2 - rect_width / 2;
    int rect_y = game_over_y + game_over_font_size + 10;
    DrawRectangleLines(rect_x, rect_y, rect_width, rect_height, WHITE);

    DrawText(player, rect_x + 10, rect_y + 10, 34, WHITE);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    Game game = {
        .score = 0,
	.game_screen = GAME,
    };

    Ship ship = init_ship((Vector2){500.0, 500.0});

    //Deque *projectiles_dq = init_deque();

    AsteroidsVector asteroids = init_asteroids_vector(MAX_ASTEROIDS);

    int player_len = 0;
    char player[128];
    player[0] = '\0';

    InitWindow(screen.width, screen.height, "Asteroids");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
	// Update
	//----------------------------------------------------------------------------------
	switch (game.game_screen) {
	case GAME: {
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

	    //if (IsKeyPressed(KEY_SPACE)) {
                //fire(&ship, projectiles_dq);
	    //}

	    /*
	    Node *projectile_node = projectiles_dq->first;
	    while (projectile_node != NULL) {
                if (projectile_node->val != NULL) {
                    Projectile *p = (Projectile *)projectile_node->val;
                    if (is_visible(p->center, p->radius, screen)) {
			Node *collision_node = NULL;

			if ((collision_node = check_projectile_asteroids_collision(p, game.asteroids, game.asteroids_len)) != NULL) {
			    Node *curr_ptr = projectile_node;

			    projectile_node = projectile_node->next;

			    remove_node(projectiles_dq, curr_ptr);

			    //remove_node(asteroids_dq, collision_node);

			    game.score++;
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
	    */
	    
	    bool asteroids_to_delete[asteroids_vector_len(asteroids)];
	    for (int i = 0; i < asteroids_vector_len(asteroids); i++) {
		asteroids_to_delete[i] = false;
	    }
	    
	    for (int i = 0; i < asteroids_vector_len(asteroids); i++) {
		Asteroid* a1 = asteroids[i];
		if (!is_on_screen(a1->center, a1->max_radius, screen)) {
		    asteroids_to_delete[i] = true;		
		}
		
		if (check_ship_asteroid_collision(&ship, a1)) {
		    game.game_screen = GAME_OVER;
		    break;
		}

		for (int j = i + 1; j < asteroids_vector_len(asteroids); j++) {
		    Asteroid* a2 = asteroids[j];
		    if (check_two_asteroids_collision(a1, a2)) {
			asteroids_to_delete[i] = true;
			asteroids_to_delete[j] = true;	       
		    }
		}
		move_asteroid(a1);	
	    }
	    
	    for(int i = asteroids_vector_len(asteroids) - 1; i >= 0 ; i--) {
		if (asteroids_to_delete[i] == true) {
		    delete_from_asteroids_vector(asteroids, i);
		}
	    }

	    if (asteroids_vector_len(asteroids) < asteroids_vector_cap(asteroids) && (GetRandomValue(0, 60) == 4)) {
                Asteroid* a;
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

		append_to_asteroids_vector(asteroids, a);
	    }

	    break;
	}
	case GAME_OVER: {
	    int key = GetKeyPressed();
	    if (key == KEY_ENTER) {
		player[player_len] = '\0';
		game.game_screen = WINNERS;

		FILE* fp = fopen("./winners.csv", "a");
		assert(fp != NULL && "Can't open winners file");

		fseek(fp, 0, SEEK_END);

		fprintf(fp, "%s,%d\n",  player, game.score);

		fclose(fp);
	    }

	    if (key == KEY_BACKSPACE) {
		if (player_len > 0) {
		    player_len--;
		    player[player_len] = '\0';
		}
	    }

	    if ((key >= 32) && (key <= 126)) {
		player[player_len] = (char) key;
		player_len++;
	    }
	    break;
	}

	case WINNERS:
	    break;
	}

        //----------------------------------------------------------------------------------

	// Draw
	//----------------------------------------------------------------------------------
	BeginDrawing();

	ClearBackground(DARKGRAY);

	switch(game.game_screen) {
	case GAME:
	    draw_ship(ship);

	    //draw_projectiles(projectiles_dq);

	    for (int i = 0; i < asteroids_vector_len(asteroids); i++) {
		draw_asteroid(asteroids[i]);
	    }

	    draw_info(0, asteroids_vector_len(asteroids), game.score);
	    break;
	case GAME_OVER:
	    draw_ship(ship);

	    //draw_projectiles(projectiles_dq);

	    for (int i = 0; i < asteroids_vector_len(asteroids); i++) {
		draw_asteroid(asteroids[i]);
	    }

	    //draw_info(projectiles_dq->count, asteroids_dq->count, game.score);

            draw_game_over(screen, player);
	    break;

	case WINNERS: {
	    int i = 0;

	    FILE* fp = fopen("./winners.csv", "r+");
	    assert(fp != NULL && "Can't open winners file");

	    char player[32];
	    int score;

	    char buffer[64];

	    while (fscanf(fp, "%[^,],%d\n", player, &score) == 2) {
		sprintf(buffer, "Player: %s - Score: %d\n", player, score);
		DrawText(buffer, 400, 300 + i * 40, 35, GREEN);
		i++;
	    }

	    fclose(fp);
	    break;
	}
	}

	EndDrawing();
	//----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    // free memory

    return 0;
    }
