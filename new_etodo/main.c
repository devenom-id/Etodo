#include <ncurses.h>
#include <json-c/json.h>
#include "json.h"
#define COLOR_BLANCO 1

int main() {
    initscr();
    start_color();
    use_default_colors();
    keypad(stdscr, 1);

    json_object* data = load_data_from_json();

    endwin();
}