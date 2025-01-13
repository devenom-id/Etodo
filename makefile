CC=gcc
CFLAGS=-lasan -ljson-c -lncurses
LFLAGS=-I/usr/include/json-c

x: main.c nsread.c
	$(CC) main.c nsread.c -o x $(CFLAGS) $(LFLAGS) -g