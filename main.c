#include <ncurses.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include "nsread.h"
#define C_AZUL 1
#define C_ROJO 2
#define C_BLANCO 3

struct Task {
  char* task;
  int state;
  char* time;
};
struct Task Task_new(char* name, int state, char* time) { struct Task task = {name, state, time}; return task; }
void Task_edit(struct Task* task, char* taskname, int state, char* time) {task->task=taskname;task->state=state;task->time=time;}
void Task_mark(struct Task* task) {task->state=!task->state;}

struct List {
  struct Task* tasks;
  int size;
};
struct List List_new() { struct List list = {NULL, 0}; return list; }
void List_add(struct List* list, struct Task task) {
  for (int i=0; i<list->size; i++) {
    if (!strcmp(list->tasks[i].task,task.task)) return;
  }
  list->size++;
  list->tasks = realloc(list->tasks, sizeof(struct Task)*list->size);
  list->tasks[list->size-1] = task;
}
void List_delete(struct List* list, int n) {
  for (int i=n+1; i<list->size; i++) {
    list->tasks[i-1] = list->tasks[i];
  }
  list->size--;
  list->tasks = realloc(list->tasks, sizeof(struct Task)*list->size);
}

struct Device {
  char* ip;
  int port;
};
struct Device* Device_new();
int Device_download();
int Device_upload();

struct UI;
typedef void (*ui_writer)(struct UI*);

struct UI {
  WINDOW** windows;
  WINDOW* main;
  ui_writer *fdraw;
  int state;
  char winarr_length;
  char fdraw_length;
  void* cbdata;
};
struct UI UI_new(WINDOW** windows, char winarr_length, WINDOW* main) {
  struct UI ui = {windows, main, NULL, -1, winarr_length, 0}; return ui;
}
void UI_register(struct UI* ui, ui_writer callback) {
  ui->fdraw_length++;
  ui->fdraw = realloc(ui->fdraw, sizeof(ui_writer)*ui->fdraw_length);
  ui->fdraw[ui->fdraw_length-1] = callback;
}
void UI_set_state(struct UI* ui, int state, void* cbdata) {
  ui->state = state; ui->cbdata = cbdata;
}
void UI_draw(struct UI* ui) { ui->fdraw[ui->state](ui); }
void UI_bring_up_all(struct UI* ui) {
  for (int i=0; i<ui->winarr_length; i++) {
    if (ui->windows[i] == ui->main) continue;
    touchwin(ui->windows[i]); wrefresh(ui->windows[i]);
  }
  touchwin(ui->main); wrefresh(ui->main);
}
void UI_bring_up(struct UI* ui) {
  touchwin(ui->main); wrefresh(ui->main);
}
void UI_bkgd(struct UI* ui, short color) { wbkgd(ui->main, COLOR_PAIR(color)); wrefresh(ui->main); }
void UI_set_main(struct UI* ui, WINDOW* win) { ui->main = win; }
void UI_free(struct UI* ui) {
  for (int i=0; i<ui->winarr_length; i++) { delwin(ui->windows[i]); }
  free(ui->fdraw);
}

void main_ui(struct UI* ui) {
  int y,x; getmaxyx(stdscr, y, x);
  attron(COLOR_PAIR(C_BLANCO));
    mvaddstr(0, x/2-3, " Etodo ");
  attroff(COLOR_PAIR(C_BLANCO));
  refresh();

  // write List
  struct List* list = ui->cbdata;
  for (int i=0; i<list->size; i++) {
    // TODO: IF DONE, SHOW HOUR OF COMPLETION
    waddstr(ui->main, list->tasks[i].state ? " (x) " : " ( ) ");
    waddstr(ui->main, list->tasks[i].task);
    waddch(ui->main, '\n');
  }

  WINDOW* optwin = ui->windows[1];
  char* optarr[][2] = {
    {" a ", " Add"},
    {" e ", " Edit"},
    {" D ", " Delete"},
    {" o ", " Move up"},
    {" l ", " Move down"},
    {" S ", " Sync"}
  };
  int n=0;
  int spacing=2;
  for (int i=0; i<6; i++) {
    int ssize = strlen(optarr[i][1])+3;
    if (n+ssize > x) break;
    wattron(optwin, COLOR_PAIR(C_BLANCO));
    mvwaddstr(optwin, 0, n, optarr[i][0]);
    wattroff(optwin, COLOR_PAIR(C_BLANCO));
    waddstr(optwin, optarr[i][1]);
    n+=ssize+spacing;
  }

  wrefresh(ui->main);
  wrefresh(ui->windows[1]);
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

void save_list(struct List* list) {
  char* home = strdup(getenv("HOME"));
  short size = strlen(home)+29;
  home = realloc(home, size+1);
  strcat(home, "/.local/share/etodo/data.json");
  FILE* file = fopen(home, "w");
  struct json_object* jobj = json_object_new_object();
  for (int i=0; i<list->size; i++) {
    struct json_object* arr = json_object_new_array();
    json_object_array_add(arr, json_object_new_int(list->tasks[i].state));
    json_object_array_add(arr, json_object_new_string(list->tasks[i].time));
    json_object_object_add(jobj, list->tasks[i].task, arr);
  }
  fputs(json_object_to_json_string(jobj), file);
  json_object_put(jobj);
  fclose(file);
  free(home);
}

struct List list_from_json(struct json_object* jobj) {
  struct List lista = List_new();
  json_object_object_foreach(jobj, key, val) {
      struct Task task = Task_new(key, 0, NULL);
      task.state = json_object_get_int(json_object_array_get_idx(val, 0));
      task.time = json_object_get_string(json_object_array_get_idx(val, 1));
      List_add(&lista, task);
  }
  return lista;
}

int main() {
  struct json_object* data = data_loader();
  struct List list = list_from_json(data);
  // draw interface
  WINDOW* stdscr = initscr();
  start_color();
  use_default_colors();
  keypad(stdscr,1);
  curs_set(0);
  init_pair(C_AZUL, 15, 33);
  init_pair(C_ROJO, 15, 124);
  init_pair(C_BLANCO, 0, 15);
  int y,x; getmaxyx(stdscr,y,x);
  WINDOW* taskwin = newwin(y-2,x,1,0);
  wrefresh(taskwin);
  WINDOW* optwin = newwin(1,x,y-1,0);
  wrefresh(optwin);

  WINDOW* winarr[2] = {taskwin, optwin};
  struct UI ui = UI_new(winarr,2,taskwin);
  UI_register(&ui, main_ui);
  UI_set_state(&ui, 0, &list);
  UI_draw(&ui);
  // handle key input
  // TODO: handle each event + terminal resize
  wgetch(ui.main);
  endwin();
  return 0;
}
