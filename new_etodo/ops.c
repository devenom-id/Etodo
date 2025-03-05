#include "json.h"
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/linkhash.h>
#include "debug.h"

void add_task(char* name, int done, char* time, json_object* jobj, char*** keys, int* keys_size) {
    json_object* arr = json_object_new_array();
    json_object_array_add(arr, json_object_new_int(done));
    json_object_array_add(arr, json_object_new_string(time));
    json_object_object_add(jobj, name, arr);
    (*keys_size)++;
    *keys = realloc(*keys, sizeof(char*)*((*keys_size)+1));
    (*keys)[(*keys_size)-1] = name;
    write_data_to_json(jobj);
}
void del_task(json_object* jobj, char*** keys, int *keys_size, int index) {
    json_object_object_del(jobj, (*keys)[index]);
    for (int i=index; i<(*keys_size-1); i++) (*keys)[i] = (*keys)[i+1];
    (*keys_size)--;
    *keys = realloc(*keys, sizeof(char*)*((*keys_size)+1));
    write_data_to_json(jobj);
}
void edit_task(int index, char* name, int done, char* time, json_object* jobj, char** keys, int keys_size) {
    struct lh_table* table = json_object_get_object(jobj);
    lh_table_lookup_entry(table, keys[index])->k = name;
    keys[index] = name;
    json_object* arr = json_object_object_get(jobj, keys[index]);
    json_object_array_insert_idx(arr, 0, json_object_new_int(done));
    json_object_array_insert_idx(arr, 1, json_object_new_string(time));
    write_data_to_json(jobj);
}
void move_task_up(int index, json_object* jobj, char** keys, int keys_size) {
    struct lh_table* table = json_object_get_object(jobj);
    pprint("table", table);
    pprint("keys", keys);
    if (index==1) {
        struct lh_entry* entry = table->head;
        table->head = entry->next;
        entry->next = table->head->next;
        table->head->next = entry;
    }
    else {
        struct lh_entry* entry = lh_table_lookup_entry(table, keys[index-2]); // A
        pprint("entry", entry);
        struct lh_entry* sup = entry->next;
        entry->next = entry->next->next; // A->C
        sup->next = entry->next->next; // B->D
        entry->next->next = sup; // C->B
    }
    char* sup2 = keys[index];
    keys[index] = keys[index-1];
    keys[index-1] = sup2;
    write_data_to_json(jobj);
}
void move_task_down(int index, json_object* jobj, char** keys, int keys_size) {
    struct lh_table* table = json_object_get_object(jobj);
    if (!index) {
        struct lh_entry* entry = table->head; // A
        table->head = entry->next; // head = B
        entry->next = table->head->next; // A -> C
        table->head->next = entry; // B -> A
    } else {
        struct lh_entry* entry = lh_table_lookup_entry(table, keys[index-1]);
        struct lh_entry* sup = entry->next;
        entry->next = entry->next->next; // A->C
        sup->next = entry->next->next; // B->D
        entry->next->next = sup; // C->B
    }
    char* sup2 = keys[index];
    keys[index] = keys[index+1];
    keys[index+1] = sup2;
    write_data_to_json(jobj);
}