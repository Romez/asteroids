.PHONY: clean

asteroids: main.c
	gcc -I./include -o asteroids main.c ./lib/libraylib.a -lm

clear:
	rm ./asteroids
