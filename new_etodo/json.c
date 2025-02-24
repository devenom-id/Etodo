#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// carga de tareas, dumpear archivo

json_object* load_data_from_json() {
    int fexists=0;
    char* path = strdup(getenv("HOME"));
    size_t path_size = strlen(path);
    path = realloc(path, path_size+30);
    strcat(path, "/.local/share/etodo/data.json");
    FILE* F = fopen(path, "r");
    char* buffer = NULL;
    if (!F) {
        path[path_size+7]=0;
        mkdir(path, 0755);
        path[path_size+7]='/';
        path[path_size+13]=0;
        mkdir(path, 0755);
        path[path_size+13]='/';
        path[path_size+20]=0;
        mkdir(path, 0755);
        path[path_size+20]='/';
        fclose(F);
        F = fopen(path, "w");
        fputs("{}", F);
        fclose(F);
        buffer = "{}";
    } else {
        fexists=1;
        struct stat st;
        stat(path, &st);
        buffer = malloc(st.st_size+1);
    }

    json_object* jobj = json_tokener_parse(buffer);
    /*heap cleanup*/
    free(path);
    if (fexists) free(buffer);
    return jobj;
}

void write_data_to_json(json_object* jobj) {
    const char* buffer = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    size_t buffer_size = strlen(buffer);
    char* path = strdup(getenv("HOME"));
    size_t path_size = strlen(path);
    path = realloc(path, path_size+30);
    strcat(path, "/.local/share/etodo/data.json");
    FILE* F = fopen(path, "w");
    fputs(buffer, F);
    fclose(F);
    /*heap cleanup*/
    free(path);
    free((void*)buffer);
}