#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>


void clrton(WINDOW* win, int a, int b, int n) {
  wmove(win, a, b);
  for (int i=0; i<n; i++) {
    waddch(win,' ');
  }
}

int nsread(WINDOW* win, char** buff, int y, int x, int width, int maxch) {
  curs_set(1);
  int length = *buff ? strlen(*buff) : 0;
  int p=0;
  int e=0;
  clrton(win, y, x, width+1);
  wmove(win,y,x);
  for (int i=0; e+i<length && i<width-p; i++) {
    waddch(win, (*buff)[e+i]);
  }
  wmove(win,y,x);
  while (1) {
    int ch = wgetch(win);
    switch (ch) {
      case '\n':
        curs_set(0);
        (*buff)[length] = 0;
        return 1;
      case 27:
        curs_set(0);
        return 0;
      case 263:
      case 127:
        if (!e) break;
        for (int i=e; i<length+1; i++) {
          (*buff)[i-1] = (*buff)[i];
        }
        *buff = realloc(*buff, length);
        // -- mod starts here --
        if (width-p+e==length && e-p) {
          clrton(win, y, x, width+1);
          wmove(win, y, x);
          e--;
          for (int i=e-p; i<width-p+e; i++) {
            waddch(win, (*buff)[i]);
          }
          length--;
        }
        else {
          length--;
          p--; e--;
          clrton(win, y, x+p, width-p);
          wmove(win, y, x+p);
          for (int i=0; e+i<length && i<width-p; i++) {
            waddch(win, (*buff)[e+i]);
          }
        }
        // -- ends here --
        wmove(win, y, x+p);
        break;
      case KEY_LEFT:
        if (!e) break;
        if (!p) {
          clrton(win, y, x, width+1);
          wmove(win, y, x);
          e--;
          for (int i=e-p; i<width-p+e; i++) {
            waddch(win, (*buff)[i]);
          }
        }
        else {e--; p--;}
        wmove(win, y, x+p);
        break;
      case KEY_RIGHT:
        if (e>=length) break;
        if (p==width) {
          clrton(win, y, x, width+1);
          wmove(win, y, x);
          e++;
          for (int i=e-p; i<width-p+e; i++) {
            waddch(win, (*buff)[i]);
          }
        }
        else {e++; p++;}
        wmove(win, y, x+p);
        break;
      default:
        if (e>=maxch) break;
        if (!(isgraph(ch) || ch==32)) break;
        *buff = realloc(*buff, length+2);
        for (int i=length; i>e; i--) {
          (*buff)[i] = (*buff)[i-1];
        }
        (*buff)[e] = ch;
        if (p==width) {
          clrton(win, y, x, width+1);
          wmove(win, y, x);
          e++;
          for (int i=e-p; i<width-p+e; i++) {
            waddch(win, (*buff)[i]);
          }
          length++;
        }
        else {
          clrton(win, y, x+p, width-p);
          wmove(win, y, x+p);
          for (int i=0; e+i<=length && i<width-p; i++) {
            waddch(win, (*buff)[e+i]);
          }
          length++;
          e++;p++;
        }
        wmove(win, y, x+p);
    }
  }
}
