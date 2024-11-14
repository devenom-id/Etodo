#include <ncurses.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json.h>
#include "nsread.h"
#define C_AZUL 1
#define C_ROJO 2
#define C_BLANCO 3

//typedef callback;
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
  void** cbdata;
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

void task_draw(WINDOW* win, struct List* list, int* p, int* e, int b, int n) {
  int y, x; getmaxyx(win, y, x);
  if (n==0) {
    wmove(win, 0, 0); wclear(win);
    int stop = (y)<=list->size ? (y) : list->size;
    for (int i=0; i<stop; i++) {
      if (list->tasks[i+b].state) { 
        mvwaddstr(win, i, 0, " (x) ");
        waddnstr(win, list->tasks[i+b].task, x-11);
        mvwaddstr(win, i, x-5, list->tasks[i+b].time);
      } else {
        mvwaddstr(win, i, 0, " ( ) ");
        waddnstr(win, list->tasks[i+b].task, x-11);
      }
    }

    wrefresh(win);
  }
  else if (n==1){
    mvwaddstr(win, *p, 5, list->tasks[*e].task);
    (*p)++;
    (*e)++;
    wattron(win, COLOR_PAIR(C_BLANCO));
    mvwaddstr(win, *p, 5, list->tasks[*e].task);
    wattroff(win, COLOR_PAIR(C_BLANCO));
    wrefresh(win);
  }
  else {
    mvwaddstr(win, *p, 5, list->tasks[*e].task);
    (*p)--;
    (*e)--;
    wattron(win, COLOR_PAIR(C_BLANCO));
    mvwaddstr(win, *p, 5, list->tasks[*e].task);
    wattroff(win, COLOR_PAIR(C_BLANCO));
    wrefresh(win);
  }
}

void main_ui(struct UI* ui) {
  int y,x; getmaxyx(stdscr, y, x);
  move(0,0);clrtoeol();
  attron(COLOR_PAIR(C_BLANCO));
    mvaddstr(0, x/2-3, " Etodo ");
  attroff(COLOR_PAIR(C_BLANCO));
  refresh();

  // write List
  task_draw(ui->main, ui->cbdata[0], NULL, NULL, 0, 0);  // draw all

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
  touchwin(ui->windows[1]);
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
  char* buffer = malloc(st.st_size+1); buffer[st.st_size]=0;
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

void mark_done(WINDOW* win, int* done) {
  *done = !(*done);
  mvwaddstr(win, 3, 16, (*done) ? "x" : " ");
}

void op_add_task(struct UI* ui) {
  struct List* list = ui->cbdata[0];
  int y; int x; getmaxyx(stdscr, y, x);
  WINDOW* win = newwin(7,40, y/2-3, x/2-20);
  keypad(win, 1);
  box(win, ACS_VLINE, ACS_HLINE);
  mvwaddstr(win, 0, 2, "New task");
  mvwaddstr(win, 2, 3, "Name: ");
  mvwaddstr(win, 3, 20-5, "( ) Is done");
  mvwaddstr(win, 5, 20-2, "[OK]");
  int opts[4][3] = {
    {2,9,20},
    {3,16,1},
    {4,15,5},
    {5,18,4}
  };
  char buffer[21] = {0};
  char* taskname = NULL;
  int done = 0;
  char* tasktime = strdup("00:00");
  wattron(win, COLOR_PAIR(C_BLANCO));
  mvwinnstr(win, opts[0][0], opts[0][1], buffer, opts[0][2]);
  mvwaddstr(win, opts[0][0], opts[0][1], buffer);
  wattroff(win, COLOR_PAIR(C_BLANCO));
  int e=0;
  while (1) {
    int ch = wgetch(win);
    switch (ch) {
      case KEY_DOWN:
        if (e==3) continue;
        mvwinnstr(win, opts[e][0], opts[e][1], buffer, opts[e][2]);
        mvwaddstr(win, opts[e][0], opts[e][1], buffer);
        if (e==1 && !done) e++;
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[e+1][0], opts[e+1][1], buffer, opts[e+1][2]);
        mvwaddstr(win, opts[e+1][0], opts[e+1][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        e++;
        break;
      case KEY_UP:
        if (!e) continue;
        mvwinnstr(win, opts[e][0], opts[e][1], buffer, opts[e][2]);
        mvwaddstr(win, opts[e][0], opts[e][1], buffer);
        if (e==3 && !done) e--;
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[e-1][0], opts[e-1][1], buffer, opts[e-1][2]);
        mvwaddstr(win, opts[e-1][0], opts[e-1][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        e--;
        break;
      case 27:
        taskname = NULL;
        goto op_add_task_out;
      case 10:
        switch (e) {
          case 0:
            nsread(win, &taskname, 2, 9, 20, 30);
            break;
          case 1:
            mark_done(win, &done);
            if (done) {mvwaddstr(win, 4, 20-5, tasktime);waddstr(win, " time");}
            else mvwhline(win, 4, 20-5, ' ', 10);
            break;
          case 2:
            time_edit(win, tasktime);
            break;
          case 3:
            goto op_add_task_out;
        }
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[e][0], opts[e][1], buffer, opts[e][2]);
        mvwaddstr(win, opts[e][0], opts[e][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        break;
    }
  }
op_add_task_out:
  delwin(win);
  UI_bring_up(ui);
  if (taskname) List_add(list, Task_new(taskname,done,tasktime));
  wmove(ui->main,0,0);
  task_draw(ui->main, ui->cbdata[0], NULL, NULL, 0, 0);
}

void op_del_task(struct UI* ui, int e) {
  struct List* list = ui->cbdata[0];
  List_delete(list, e);
  task_draw(ui->main, list, NULL, NULL, 0, 0);
}
void op_ren_task(struct UI* ui, const int e) {
  struct List* list = ui->cbdata[0];
  int y; int x; getmaxyx(stdscr, y, x);
  WINDOW* win = newwin(7,40, y/2-3, x/2-20);
  keypad(win, 1);
  box(win, ACS_VLINE, ACS_HLINE);
  mvwaddstr(win, 0, 2, "New task");
  mvwaddstr(win, 2, 3, "Name: ");
  mvwaddstr(win, 3, 20-5, "( ) Is done");
  mvwaddstr(win, 5, 20-2, "[OK]");
  int opts[4][3] = {
    {2,9,20},
    {3,16,1},
    {4,15,5},
    {5,18,4}
  };
  char buffer[21] = {0};
  char** taskname = &list->tasks[e].task;
  char* tasktime = list->tasks[e].time;
  char* namebkp = strdup(*taskname);
  wattron(win, COLOR_PAIR(C_BLANCO));
  mvwaddstr(win, opts[0][0], opts[0][1], list->tasks[e].task);
  wattroff(win, COLOR_PAIR(C_BLANCO));
  mvwaddstr(win, opts[1][0], opts[1][1], list->tasks[e].state?"x":" ");
  if (list->tasks[e].state) {
    mvwaddstr(win, 4, 20-5, list->tasks[e].time);
    waddstr(win, " time");
  }
  int p=0;
  while (1) {
    int ch = wgetch(win);
    switch (ch) {
      case KEY_DOWN:
        if (p==3) continue;
        mvwinnstr(win, opts[p][0], opts[p][1], buffer, opts[p][2]);
        mvwaddstr(win, opts[p][0], opts[p][1], buffer);
        if (p==1 && !list->tasks[e].state) p++;
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[p+1][0], opts[p+1][1], buffer, opts[p+1][2]);
        mvwaddstr(win, opts[p+1][0], opts[p+1][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        p++;
        break;
      case KEY_UP:
        if (!p) continue;
        mvwinnstr(win, opts[p][0], opts[p][1], buffer, opts[p][2]);
        mvwaddstr(win, opts[p][0], opts[p][1], buffer);
        if (p==3 && !list->tasks[e].state) p--;
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[p-1][0], opts[p-1][1], buffer, opts[p-1][2]);
        mvwaddstr(win, opts[p-1][0], opts[p-1][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        p--;
        break;
      case 27:
        *taskname = NULL;
        goto op_ren_task_out;
      case 10:
        switch (p) {
          case 0:
            nsread(win, taskname, 2, 9, 20, 30);
            break;
          case 1:
            mark_done(win, &list->tasks[e].state);
            if (list->tasks[e].state) {mvwaddstr(win, 4, 20-5, tasktime);waddstr(win, " time");}
            else mvwhline(win, 4, 20-5, ' ', 10);
            break;
          case 2:
            time_edit(win, tasktime);
            break;
          case 3:
            goto op_ren_task_out;
        }
        wattron(win, COLOR_PAIR(C_BLANCO));
        mvwinnstr(win, opts[p][0], opts[p][1], buffer, opts[p][2]);
        mvwaddstr(win, opts[p][0], opts[p][1], buffer);
        wattroff(win, COLOR_PAIR(C_BLANCO));
        break;
    }
  }
op_ren_task_out:
  delwin(win);
  UI_bring_up(ui);
  if (!(*taskname)) *taskname = namebkp;
  List_add(list, Task_new(*taskname,list->tasks[e].state,tasktime));
  wmove(ui->main,0,0);
  task_draw(ui->main, ui->cbdata[0], NULL, NULL, 0, 0);
}
void op_reorder_up(struct UI* ui, int* p, int* e, int b) {
  struct List* list = ui->cbdata[0];
  if (!*e) return;
  struct Task aux = list->tasks[*e];
  list->tasks[*e] = list->tasks[*e-1];
  list->tasks[*e-1] = aux;
  wmove(ui->main,0,0);
  task_draw(ui->main, ui->cbdata[0], NULL, NULL, b, 0);
  (*p)--;(*e)--;
}
void op_reorder_down(struct UI* ui, int* p, int* e, int b) {
  struct List* list = ui->cbdata[0];
  if (*e==list->size-1) return;
  struct Task aux = list->tasks[*e];
  list->tasks[*e] = list->tasks[*e+1];
  list->tasks[*e+1] = aux;
  wmove(ui->main,0,0);
  task_draw(ui->main, ui->cbdata[0], NULL, NULL, b, 0);
  (*p)++;(*e)++;
}
void op_mark_task(struct UI* ui, int p, int e) {
  struct List* list = ui->cbdata[0];
  list->tasks[e].state = !list->tasks[e].state;
  int x=getmaxx(ui->main);
  if (list->tasks[e].state) {
    time_t tm=time(0);
    struct tm* now = localtime(&tm);
    char buff[7]={0};
    strftime(buff, 6, "%H:%M", now);
    strcpy(list->tasks[e].time, buff);
    mvwaddstr(ui->main, p, 2, "x");
    mvwaddstr(ui->main, p, x-5, buff);
  } else {
    mvwaddstr(ui->main, p, 2, " ");
    wmove(ui->main, p, x-5);wclrtoeol(ui->main);
  }
}

int task_nav(struct UI* ui) {
  int p = 0;
  int e = 0;
  int b = 0;
  int y = getmaxy(ui->main);
  struct List* list = ui->cbdata[0];
  ui->cbdata[1] = &p;
  ui->cbdata[2] = &e;
  if (list->size) {
    wattron(ui->main, COLOR_PAIR(C_BLANCO));
    mvwaddstr(ui->main, 0, 5, list->tasks[0].task);
    wattroff(ui->main, COLOR_PAIR(C_BLANCO));
  }
  while (1) {
    int ch = wgetch(ui->main);
    switch (ch) {
      case KEY_RESIZE: {
        endwin();refresh();
        int y, x;getmaxyx(stdscr, y, x);
        clear();
        mvaddstr(0, x/2-3, " ETODO ");refresh();
        wresize(ui->main, y-2, x);
        mvwin(ui->main, 1, 0);
        wresize(ui->windows[1], 1, x);
        mvwin(ui->windows[1], y-1, 0);
        UI_draw(ui);
        }
        y = getmaxy(ui->main);
        break;
      case KEY_DOWN:
        if (!list->size||e==list->size-1) break;
        if (p==y-1&&e!=list->size-1) {
          b++;
          task_draw(ui->main, list, &p, &e, b, 0);
          e++;
          break;
        }
        task_draw(ui->main, list, &p, &e, b, 1);
        continue;
      case KEY_UP:
        if (!list->size||!e) break;
        if (!p&&e) {
          b--;
          task_draw(ui->main, list, &p, &e, b, 0);
          e--;
          break;
        }
        task_draw(ui->main, list, &p, &e, b, -1);
        continue;
      case 'a':
        op_add_task(ui);
        save_list(list);
        break;
      case 'D':
        if (!list->size) continue;
        op_del_task(ui, e);
        save_list(list);
        if (!list->size) continue;
        if (e==list->size) {p--;e--;}
        break;
      case 'e':
        op_ren_task(ui, e);
        save_list(list);
        break;
      case 'o': // TODO: Reorder up
        if (!p&&e) {b--;p++;}
        op_reorder_up(ui, &p, &e, b);
        save_list(list);
        break;
      case 'l': // TODO: Reorder down
        if (p==y-1&&e!=list->size-1) {b++;p--;}
        op_reorder_down(ui, &p, &e, b);
        save_list(list);
        break;
      case 10:
        if (list->size) op_mark_task(ui, p, e);
        save_list(list);
        continue;
      case 27:
        return 1;
    }
    if (!list->size) continue;
    wattron(ui->main, COLOR_PAIR(C_BLANCO));
    mvwaddstr(ui->main, p, 5, list->tasks[e].task);
    wattroff(ui->main, COLOR_PAIR(C_BLANCO));
  }
}

int main() {
  struct json_object* data = data_loader();
  struct List list = list_from_json(data);
  // draw interface
  WINDOW* stdscr = initscr();
  start_color();
  use_default_colors();
  set_escdelay(100);
  noecho();
  curs_set(0);
  init_pair(C_AZUL, 15, 33);
  init_pair(C_ROJO, 15, 124);
  init_pair(C_BLANCO, 0, 15);
  int y,x; getmaxyx(stdscr,y,x);
  WINDOW* taskwin = newwin(y-2,x,1,0);
  keypad(taskwin, 1);
  scrollok(taskwin, 1);
  wrefresh(taskwin);
  WINDOW* optwin = newwin(1,x,y-1,0);
  wrefresh(optwin);

  WINDOW* winarr[2] = {taskwin, optwin};
  struct UI ui = UI_new(winarr,2,taskwin);
  UI_register(&ui, main_ui);
  int p=0; int e=0;
  void* cbdata[3] = {&list, &p, &e};
  UI_set_state(&ui, 0, cbdata);
  UI_draw(&ui);
  // handle key input
  // TODO: terminal resize
  task_nav(&ui);
  endwin();
  return 0;
}
