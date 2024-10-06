#include <ncurses.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#define C_AZUL 1
#define C_ROJO 2
#define C_BLANCO 3

struct Task {
  char* task;
  int state;
  int time;
};
struct Task* Task_new();  // zero the task
void Task_edit(struct Task* task, char* taskname, int state, int time);
void Task_mark(struct Task* task, int state);

struct List {
  struct Task* tasks;
  int size;
};
struct List* List_new();  // zero the list
void List_add(struct List* list, char* task, int state, int time);
void List_delete(struct List* list, struct Task* task);

struct Device {
  char* ip;
  int port;
};
struct Device* Device_new();
int Device_download();
int Device_upload();

struct UI {
  WINDOW** windows;
  WINDOW* main;
  int state;
  char winarr_length;
  void (*fdraw)(struct UI*);
};
struct UI UI_new(WINDOW** windows, char winarr_length, WINDOW* main, int state) {
  struct UI ui = {windows, main, state, winarr_length}; return ui;
}
void UI_register(struct UI* ui, void (*callback)(struct UI*)) {
  ui->fdraw = callback;
}
void UI_draw(struct UI* ui) { ui->fdraw(ui); }
void UI_bkgd(struct UI* ui, short color) { wbkgd(ui->main, COLOR_PAIR(color)); wrefresh(ui->main); }
void UI_set_main(struct UI* ui, WINDOW* win) { ui->main = win; }
void UI_set_state(struct UI* ui, int state) { ui->state = state; }
void UI_free(struct UI* ui) {
  for (int i=0; i<ui->winarr_length; i++) {
    delwin(ui->windows[i]);
  }
}

void main_ui(struct UI* ui) {
  int y,x; getmaxyx(ui->main, y, x);
  mvwaddstr(ui->main, 0, x/2-2, "Etodo");
  wrefresh(ui->main);
}

struct json_object* data_loader() {
  char* home = strdup(getenv("HOME"));
  short size = strlen(home)+20;
  home = realloc(home, size+1);
  strcat(home, "/.local/share/etodo/");
  char* ptr=home;
  while (1) {
    for (; *ptr!='/'; ptr++) {
      if (!(*ptr)) goto loader_out;
    }
    *ptr=0;
    mkdir(home, 0755);
    *ptr='/';
    ptr++;
  }
loader_out:
  size += 9;
  home = realloc(home, size+1);
  strcat(home, "data.json");
  FILE* file = fopen(home, "r");
  if (!file) {
    file = fopen(home, "w");
    fputs("{}", file);
    fclose(file);
    file = fopen(home, "r");
  }
  struct stat st; stat(home, &st);
  char* buffer = malloc(st.st_size);
  fread(buffer, 1, st.st_size, file);
  struct json_object* jobj = json_tokener_parse(buffer);
  fclose(file);
  free(home);
  free(buffer);
  return jobj;
}

int main() {
  struct json_object* data = data_loader();
  // draw interface
  WINDOW* stdscr = initscr();
  start_color();
  use_default_colors();
  keypad(stdscr,1);
  curs_set(0);
  init_pair(C_AZUL, 15, 33);
  init_pair(C_ROJO, 15, 124);
  int y,x; getmaxyx(stdscr,y,x);
  WINDOW* taskwin = newwin(y-1,x, 0, 0);
  wrefresh(taskwin);
  WINDOW* optwin = newwin(1,x,y-1,0);
  wrefresh(optwin);

  WINDOW* winarr[2] = {taskwin, optwin};
  struct UI ui = UI_new(winarr,2,taskwin,1);
  UI_register(&ui, main_ui);
  UI_draw(&ui);
  // handle key input
  wgetch(ui.main);
  endwin();
  return 0;
}
