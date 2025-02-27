#include <json-c/json_object.h>
#include <json-c/linkhash.h>
#include <ncurses.h>
#include <json-c/json.h>

void list_tasks(WINDOW* win, json_object* jobj, char** keys, int keys_size) {
    int y, x; getmaxyx(win, y, x);
    for (int i=0; i<y && i!=keys_size; i++) {
        json_object* arr = json_object_object_get(jobj, keys[i]);
        int done = json_object_get_int(json_object_array_get_idx(arr, 0));
        const char* dtime = json_object_get_string(json_object_array_get_idx(arr, 1));
        mvwaddstr(win, i, 0, "( )");
        if (done) {mvwaddch(win, i, 1, 'x'); mvwaddstr(win, i, x-5, dtime);}
        mvwaddstr(win, i, 4, keys[i]);
    }
}

void list_interaction(WINDOW* win, json_object* jobj, char** keys, int keys_size) {
    int p = 0;
    int e = 0;
    while (1) {
        int ch = wgetch(win);
        switch (ch) {
            case KEY_DOWN:
                // check bad case: continue because of it.
                // rewrite without highlight
                // add to p & e
                // rewrite option with highlight
            case KEY_UP:
            case 27:
                // exit
            case 10:
                // mark
            case 'a':
                // add task
            case 'D':
                // delete task
            case 'e':
                // edit task
            case 'l':
                // put down
            case 'o':
                // put up
            case 'S':
                // synchronize
                break;
        }
    }
}