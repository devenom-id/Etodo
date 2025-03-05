#include <stdio.h>
FILE* FDEBUG = NULL;
void open_debug_file() {
    FDEBUG = fopen("DEBUG", "w");
}
void pprint(char* n, void* p) {
    fprintf(FDEBUG, "%s: %p\n", n, p);
    fflush(FDEBUG);
}