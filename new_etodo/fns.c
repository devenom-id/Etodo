#include <ncurses.h>
#include <stdio.h>
#include "fns.h"

void mark_done(WINDOW* win, int* done) {
    *done = !(*done);
    mvwaddstr(win, 3, 16, (*done) ? "x" : " ");
}
int time_input_validation(char* time) {
  char hour = (time[0]-'0')*10+time[1]-'0';
  char minute = (time[3]-'0')*10+time[4]-'0';
  if (hour > 23 || minute > 59) return 0;
  return 1;
}
void time_edit(WINDOW* win, char* time) {
    int e=0;
    mvwaddstr(win, 4, 15, time);
    wattron(win, COLOR_PAIR(C_BLANCO));
    mvwaddch(win, 4, 15+e, time[e]);
    wattroff(win, COLOR_PAIR(C_BLANCO));
    while (1) {
        int ch = wgetch(win);
        switch (ch) {
            case KEY_LEFT:
                if (!e) break;
                mvwaddch(win, 4, 15+e, time[e]);
                if (e==3) e-=2;
                else e--;
                wattron(win, COLOR_PAIR(C_BLANCO));
                mvwaddch(win, 4, 15+e, time[e]);
                wattroff(win, COLOR_PAIR(C_BLANCO));
                break;
        case KEY_RIGHT:
            if (e==4) break;
            mvwaddch(win, 4, 15+e, time[e]);
            if (e==1) e+=2;
            else e++;
            wattron(win, COLOR_PAIR(C_BLANCO));
            mvwaddch(win, 4, 15+e, time[e]);
            wattroff(win, COLOR_PAIR(C_BLANCO));
            break;
        case 10:
            if (!time_input_validation(time)) {
                int y,x; getmaxyx(stdscr,y,x);
                WINDOW* cwin = newwin(3,28,y/2-1,x/2-14);
                mvwaddstr(cwin, 0, x/2-2, "Error");
                mvwaddstr(cwin, 1, 1, "Formato de tiempo invalido");
                mvwaddstr(cwin, 2, x/2-2, "[OK]");
                while (1) {
                    int ch = wgetch(cwin);
                    if (ch==10||ch==27) break;
                }
                delwin(cwin);
                touchwin(win);
                break;
            }
            return;
        case 27:
            return;
        default:
            if (!(ch>='0' && ch<='9')) break;
            time[e] = ch;
            wattron(win, COLOR_PAIR(C_BLANCO));
            mvwaddch(win, 4, 15+e, ch);
            wattroff(win, COLOR_PAIR(C_BLANCO));
        }
    }
}