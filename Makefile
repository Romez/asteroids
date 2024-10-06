.PHONY: clean

asteroids: main.c asteroids.c polar.c projectiles.c
	gcc -std=c2x -Wall -pedantic -I./include main.c asteroids.c polar.c projectiles.c -o asteroids ./lib/libraylib.a -lm

clear:
	rm ./asteroids
