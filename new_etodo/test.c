#include <json-c/json_object.h>
#include <json-c/json_types.h>
#include <json-c/linkhash.h>
#include <stdio.h>
#include <json-c/json.h>
#include <ncurses.h>
#include "json.h"
#include "ui.h"
#include "ops.h"

int main() {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, 0, 15);
    keypad(stdscr, 1);
    set_escdelay(200);
    curs_set(0); noecho();
    json_object* jobj = load_data_from_json();
    char** rarr;
    int rarr_size = json_get_keys(jobj, &rarr);
    list_tasks(stdscr, jobj, rarr, rarr_size);
    list_interaction(stdscr, jobj, &rarr, &rarr_size);
    endwin();
}


/*int main() {
    json_object* jobj = load_data_from_json();
    char** rarr;
    int rarr_size = json_get_keys(jobj, &rarr);
    puts(json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY));
    move_task_down(0, jobj, rarr, rarr_size);
    puts(json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY));
}*/


/*int main() {
    json_object* jobj = load_data_from_json();
    char** rarr;
    int rarr_size = json_get_keys(jobj, &rarr);
    for (int i=0; i<rarr_size; i++) {
        puts(rarr[i]);
    }
    puts("----");
    del_task(jobj, &rarr, &rarr_size, 7);
    for (int i=0; i<rarr_size; i++) {
        puts(rarr[i]);
    }
    printf("%d\n", rarr_size);
}*/