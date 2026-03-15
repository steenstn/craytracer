all:
	gcc -std=c99 -O3 -ffast-math -fopenmp -march=native -Wall -Werror main.c -o craytracer -lm -lSDL2
