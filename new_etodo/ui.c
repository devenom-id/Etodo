#include <json-c/json_object.h>
#include <json-c/linkhash.h>
#include <ncurses.h>
#include <json-c/json.h>

void list_tasks(WINDOW* win, json_object* jobj) {
    int y, x; getmaxyx(win, y, x);
    struct lh_entry* entry = json_object_get_object(jobj)->head;
    for (int i=0; i<y && entry; i++) {
        const char* name = entry->k;
        int done = json_object_get_int(json_object_array_get_idx(entry->v, 0));
        const char* dtime = json_object_get_string(json_object_array_get_idx(entry->v, 1));
        mvwaddstr(win, i, 0, "( )");
        if (done) {mvwaddch(win, i, 1, 'x'); mvwaddstr(win, i, x-5, dtime);}
        mvwaddstr(win, i, 4, name);
        entry=entry->next;
    }
}