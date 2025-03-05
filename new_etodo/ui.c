#include "json.h"
#include <json-c/json_object.h>
#include <json-c/linkhash.h>
#include <ncurses.h>
#include <string.h>
#include "nsread.h"
#include "fns.h"
#include "ops.h"
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

void ui_add_task(WINDOW* win, json_object* jobj, char*** keys, int* keys_size) {
    int y,x; getmaxyx(stdscr, y, x);
    WINDOW* wobj = newwin(7,40, y/2-3, x/2-20);
    keypad(wobj, 1);
    box(wobj, ACS_VLINE, ACS_HLINE);
    mvwaddstr(wobj, 0, 2, "New task");
    mvwaddstr(wobj, 2, 3, "Name: ");
    mvwaddstr(wobj, 3, 20-5, "( ) Is done");
    mvwaddstr(wobj, 5, 20-2, "[OK]");
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
    wattron(wobj, COLOR_PAIR(C_BLANCO));
    mvwinnstr(wobj, opts[0][0], opts[0][1], buffer, opts[0][2]);
    mvwaddstr(wobj, opts[0][0], opts[0][1], buffer);
    wattroff(wobj, COLOR_PAIR(C_BLANCO));
    int e=0;

    while (1) {
        int ch = wgetch(wobj);
        switch (ch) {
            case KEY_DOWN:
                if (e==3) continue;
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                if (e==1 && !done) e++;
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e+1][0], opts[e+1][1], buffer, opts[e+1][2]);
                mvwaddstr(wobj, opts[e+1][0], opts[e+1][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                e++;
                break;
            case KEY_UP:
                if (!e) continue;
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                if (e==3 && !done) e--;
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e-1][0], opts[e-1][1], buffer, opts[e-1][2]);
                mvwaddstr(wobj, opts[e-1][0], opts[e-1][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                e--;
                break;
            case 27:
                taskname = NULL;
                goto ui_add_task_out;
            case 10:
                switch (e) {
                    case 0:
                        nsread(wobj, &taskname, 2, 9, 20, 30);
                        break;
                    case 1:
                        mark_done(wobj, &done);
                        if (done) {mvwaddstr(wobj, 4, 20-5, tasktime);waddstr(wobj, " time");}
                        else mvwhline(wobj, 4, 20-5, ' ', 10);
                        break;
                    case 2:
                        time_edit(wobj, tasktime);
                        break;
                    case 3:
                        goto ui_add_task_out;
                }
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                break;
        }
    }
ui_add_task_out:
    delwin(wobj);
    touchwin(win);
    wrefresh(win);
    if (taskname) {
        add_task(taskname, done, tasktime, jobj, keys, keys_size);
    }
    write_data_to_json(jobj);
}
void ui_edit_task(WINDOW* win, json_object* jobj, char** keys, int keys_size, int index) {
    int y,x; getmaxyx(stdscr, y, x);
    WINDOW* wobj = newwin(7,40, y/2-3, x/2-20);
    keypad(wobj, 1);
    box(wobj, ACS_VLINE, ACS_HLINE);
    json_object* arr = json_object_object_get(jobj, keys[index]);
    char* taskname = keys[index];
    int done = json_object_get_int(json_object_array_get_idx(arr, 0));
    char* tasktime = (char*)json_object_get_string(json_object_array_get_idx(arr, 1));
    mvwaddstr(wobj, 0, 2, "New task");
    mvwaddstr(wobj, 2, 3, "Name: ");
    mvwaddstr(wobj, 2, 9, keys[index]);
    mvwaddstr(wobj, 3, 20-5, "( ) Is done");
    if (done) {
        mvwaddch(wobj, 3, 16, 'x');
        mvwaddstr(wobj, 4, 20-5, tasktime);waddstr(wobj, " time");
    }
    mvwaddstr(wobj, 5, 20-2, "[OK]");
    int opts[4][3] = {
        {2,9,20},
        {3,16,1},
        {4,15,5},
        {5,18,4}
    };
    char buffer[21] = {0};
    wattron(wobj, COLOR_PAIR(C_BLANCO));
    mvwinnstr(wobj, opts[0][0], opts[0][1], buffer, opts[0][2]);
    mvwaddstr(wobj, opts[0][0], opts[0][1], buffer);
    wattroff(wobj, COLOR_PAIR(C_BLANCO));
    int e=0;

    while (1) {
        int ch = wgetch(wobj);
        switch (ch) {
            case KEY_DOWN:
                if (e==3) continue;
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                if (e==1 && !done) e++;
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e+1][0], opts[e+1][1], buffer, opts[e+1][2]);
                mvwaddstr(wobj, opts[e+1][0], opts[e+1][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                e++;
                break;
            case KEY_UP:
                if (!e) continue;
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                if (e==3 && !done) e--;
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e-1][0], opts[e-1][1], buffer, opts[e-1][2]);
                mvwaddstr(wobj, opts[e-1][0], opts[e-1][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                e--;
                break;
            case 27:
                taskname = NULL;
                goto ui_edit_task_out;
            case 10:
                switch (e) {
                    case 0:
                        nsread(wobj, &taskname, 2, 9, 20, 30);
                        break;
                    case 1:
                        mark_done(wobj, &done);
                        if (done) {mvwaddstr(wobj, 4, 20-5, tasktime);waddstr(wobj, " time");}
                        else mvwhline(wobj, 4, 20-5, ' ', 10);
                        break;
                    case 2:
                        time_edit(wobj, tasktime);
                        break;
                    case 3:
                        goto ui_edit_task_out;
                }
                wattron(wobj, COLOR_PAIR(C_BLANCO));
                mvwinnstr(wobj, opts[e][0], opts[e][1], buffer, opts[e][2]);
                mvwaddstr(wobj, opts[e][0], opts[e][1], buffer);
                wattroff(wobj, COLOR_PAIR(C_BLANCO));
                break;
        }
    }
ui_edit_task_out:
    delwin(wobj);
    touchwin(win);
    wrefresh(win);
    if (taskname) {
        edit_task(index, taskname, done, tasktime, jobj, keys, keys_size);
    }
    write_data_to_json(jobj);
}

void ui_sync_list() {}

void list_interaction(WINDOW* win, json_object* jobj, char*** keys, int* keys_size) {
    int p = 0;
    int e = 0;
    int y,x; getmaxyx(win, y, x);
    wattron(win, COLOR_PAIR(C_BLANCO));
    mvwaddstr(win, 0, 4, (*keys)[0]);
    wattroff(win, COLOR_PAIR(C_BLANCO));
    wrefresh(win);
    while (1) {
        int ch = wgetch(win);
        switch (ch) {
            case KEY_DOWN:
                // check bad case: continue because of it.
                if (p == y-1 || e == (*keys_size)-1) continue;
                // rewrite without highlight
                mvwaddstr(win, p, 4, (*keys)[e]);
                // add to p & e
                p++; e++;
                // rewrite option with highlight
                wattron(win, COLOR_PAIR(C_BLANCO));
                mvwaddstr(win, p, 4, (*keys)[e]);
                wattroff(win, COLOR_PAIR(C_BLANCO));
                break;
            case KEY_UP:
                // check bad case: continue because of it.
                if (!p || !e) continue;
                // rewrite without highlight
                mvwaddstr(win, p, 4, (*keys)[e]);
                // add to p & e
                p--; e--;
                // rewrite option with highlight
                wattron(win, COLOR_PAIR(C_BLANCO));
                mvwaddstr(win, p, 4, (*keys)[e]);
                wattroff(win, COLOR_PAIR(C_BLANCO));
                break;
            case 27:
                return;
            case 10:
                // mark
                break;
            case 'a':
                ui_add_task(win, jobj, keys, keys_size);
                werase(win);
                list_tasks(win, jobj, *keys, *keys_size);
                break;
            case 'D':
                // Consider adding wdeleteln(3)
                if (!*keys_size) continue;
                del_task(jobj, keys, keys_size, e);
                werase(win);
                list_tasks(win, jobj, *keys, *keys_size);
                if (e==*keys_size) {p--;e--;}
                break;
            case 'e':
                {if (!*keys_size) continue;
                ui_edit_task(win, jobj, *keys, *keys_size, e);
                wmove(win, p, 0); wclrtoeol(win);
                mvwaddstr(win, p, 0, "( )");
                wattron(win, COLOR_PAIR(C_BLANCO));
                mvwaddstr(win, p, 4, (*keys)[e]);
                wattroff(win, COLOR_PAIR(C_BLANCO));
                json_object* arr = json_object_object_get(jobj, (*keys)[e]);
                if (json_object_get_int(json_object_array_get_idx(arr, 0))) {
                    mvwaddch(win, p, 1, 'x'); mvwaddstr(win, p, x-5, json_object_get_string(json_object_array_get_idx(arr, 1)));
                }
                break;}
            case 'l':
                {if (!*keys_size || e == (*keys_size-1)) continue;
                wmove(win,p,0);wclrtoeol(win);
                mvwaddstr(win, p, 0, "( )");
                mvwaddstr(win, p, 4, (*keys)[e+1]);
                json_object* arr = json_object_object_get(jobj, (*keys)[e+1]);
                if (json_object_get_int(json_object_array_get_idx(arr, 0))) {
                    mvwaddch(win, p, 1, 'x'); mvwaddstr(win, p, x-5, json_object_get_string(json_object_array_get_idx(arr, 1)));
                }
                wmove(win,p+1,0);wclrtoeol(win);
                mvwaddstr(win, p+1, 0, "( )");
                mvwaddstr(win, p+1, 4, (*keys)[e]);
                arr = json_object_object_get(jobj, (*keys)[e]);
                if (json_object_get_int(json_object_array_get_idx(arr, 0))) {
                    mvwaddch(win, p+1, 1, 'x'); mvwaddstr(win, p+1, x-5, json_object_get_string(json_object_array_get_idx(arr, 1)));
                }
                move_task_down(e, jobj, *keys, *keys_size);
                p++; e++;
                // check if in last element and check if no elements|same in other cases
                break;}
            case 'o':
                {if (!*keys_size || !e) continue;
                wmove(win,p,0);wclrtoeol(win);
                mvwaddstr(win, p, 0, "( )");
                mvwaddstr(win, p, 4, (*keys)[e-1]);
                json_object* arr = json_object_object_get(jobj, (*keys)[e-1]);
                if (json_object_get_int(json_object_array_get_idx(arr, 0))) {
                    mvwaddch(win, p, 1, 'x'); mvwaddstr(win, p, x-5, json_object_get_string(json_object_array_get_idx(arr, 1)));
                }
                wmove(win,p-1,0);wclrtoeol(win);
                mvwaddstr(win, p-1, 0, "( )");
                mvwaddstr(win, p-1, 4, (*keys)[e]);
                arr = json_object_object_get(jobj, (*keys)[e]);
                if (json_object_get_int(json_object_array_get_idx(arr, 0))) {
                    mvwaddch(win, p-1, 1, 'x'); mvwaddstr(win, p-1, x-5, json_object_get_string(json_object_array_get_idx(arr, 1)));
                }
                move_task_up(e, jobj, *keys, *keys_size);
                p--; e--;
                break;}
            case 'S':
                // synchronize
                break;
        }
    }
}