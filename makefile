CC=gcc
CFLAGS=-lasan -ljson-c -lncurses
LFLAGS=-I/usr/include/json-c

etodo: main.c nsread.c
	$(CC) main.c nsread.c -o etodo $(CFLAGS) $(LFLAGS) -g
