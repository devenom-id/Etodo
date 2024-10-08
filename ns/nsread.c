#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#define C_BLANCO 1

void nsread(WINDOW* win, char** buff, int y, int x, int width, int maxch) {
  int length = *buff ? strlen(*buff) : 0;
  int p=0;
  int e=0;
  wmove(win,y,x);
  while (1) {
    int ch = wgetch(win);
    switch (ch) {
      case '\n':
      case 27:
        return;
      case 263:
        for (int i=e; i<length+1; i++) {
          (*buff)[i-1] = (*buff)[i];
        }
        *buff = realloc(*buff, length);
        length--;
        p--; e--;
        wmove(win, y, x+p);
        for (int i=e; i<length; i++) { waddch(win, (*buff)[i]); }
        break;
      case KEY_LEFT:
      case KEY_RIGHT:
      default:
        if (isgraph(ch) || ch==32) {
          *buff = realloc(*buff, length+2);
          (*buff)[length] = ch;
          length++;
          p++; e++;
          waddch(win, ch);
        }
    }
  }
}

int main() {
  initscr();
  start_color();
  use_default_colors();
  noecho();
  keypad(stdscr, 1);
  char* buff = NULL;
  nsread(stdscr, &buff, 0, 4, 7, 15);
  endwin();
}
