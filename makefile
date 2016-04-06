simulator: simulator.c
	gcc -o simulator simulator.c -std=gnu11 -g -O0 -Wall -Werror

all:
	gcc -o simulator simulator.c -std=gnu11 -g -O0 -Wall -Werror

clean:
	rm simulator