#ifndef FNS_H
#define FNS_H
#define C_BLANCO 1
#include <ncurses.h>
void mark_done(WINDOW* win, int* done);
int time_input_validation(char* time);
void time_edit(WINDOW* win, char* time);
#endif