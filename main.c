#include <ncurses.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>

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
  // check dirs and file on $HOME/.local/share/etodo/data.json
  struct json_object* data = data_loader();
  // load file or create an empty one
  // draw interface
  // handle key input
  return 0;
}
