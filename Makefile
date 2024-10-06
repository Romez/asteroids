.PHONY: clean

asteroids: main.c asteroids.c polar.c
	gcc -std=c2x -Wall -pedantic -I./include main.c asteroids.c polar.c -o asteroids ./lib/libraylib.a -lm

clear:
	rm ./asteroids
