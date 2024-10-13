#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include "nsread.h"

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
        // -- mod starts here --
        if (width-p+e==length && e-p) {
          wmove(win, y, x);
          wclrtoeol(win);
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
          wmove(win, y, x+p);
          wclrtoeol(win);
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
          wmove(win, y, x);
          wclrtoeol(win);
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
          wmove(win, y, x);
          wclrtoeol(win);
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
          wmove(win, y, x);
          wclrtoeol(win);
          wmove(win, y, x);
          e++;
          for (int i=e-p; i<width-p+e; i++) {
            waddch(win, (*buff)[i]);
          }
          length++;
        }
        else {
          wmove(win, y, x+p);
          wclrtoeol(win);
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
