#ifndef OPS_H
#define OPS_H
#include <json-c/json.h>
void add_task(char* name, int done, char* time, json_object* jobj, char*** keys, int* keys_size);
void del_task(json_object* jobj, char*** keys, int *keys_size, int index);
void edit_task(int index, char* name, int done, char* time, json_object* jobj, char** keys, int keys_size);
void move_task_up(int index, json_object* jobj, char** keys, int keys_size);
void move_task_down(int index, json_object* jobj, char** keys, int keys_size);
#endif