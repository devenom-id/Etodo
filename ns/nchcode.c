#include <ncurses.h>

int main() {
  initscr();
  keypad(stdscr,1);
  int ch = getch();
  endwin();
  printf("%d\n", ch);
}
