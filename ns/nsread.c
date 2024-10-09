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
      case 127:
        if (!e) break;
        for (int i=e; i<length+1; i++) {
          (*buff)[i-1] = (*buff)[i];
        }
        *buff = realloc(*buff, length);
        length--;
        p--; e--;
        wmove(win, y, x+p);
        wclrtoeol(win);
        wmove(win, y, x+p);
        for (int i=0; e<length && i<width-p; i++) {
          waddch(win, (*buff)[e+i]);
        }
        break;
      case KEY_LEFT:
        if (!e) break;
        e--; p--;
        wmove(win, y, x+p);
        break;
      case KEY_RIGHT:
        if (e>=length) break;
        e++; p++;
        wmove(win, y, x+p);
        break;
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
  WINDOW* win = newwin(1,8,0,4);
  keypad(win, 1);
  init_pair(1, 160, 15);
  wbkgd(win, COLOR_PAIR(C_BLANCO));
  char* buff = NULL;
  nsread(win, &buff, 0, 0, 7, 15);
  endwin();
  printf("buff: %s\n", buff);
}
