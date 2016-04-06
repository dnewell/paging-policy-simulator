CC=gcc
CFLAGS=-I. -std=gnu11 -g -O0 -Wall 

%.O: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

simulator: simulator.c
	gcc -o simulator simulator.c -I. -std=gnu11 -g -O0 -Wall