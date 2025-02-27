#ifndef UI_H
#define UI_H
#include <ncurses.h>
#include <json-c/json.h>
void list_tasks(WINDOW* win, json_object* jobj, char** keys, int keys_size);
#endif