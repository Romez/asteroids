.PHONY: clean

asteroids: main.c
	gcc -std=c2x -Wall -pedantic -I./include main.c deque.c -o asteroids ./lib/libraylib.a -lm

clear:
	rm ./asteroids
