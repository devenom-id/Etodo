#ifndef JSON_H
#define JSON_H
#include <json-c/json.h>
json_object* load_data_from_json();
void write_data_to_json(json_object* jobj);
int json_get_keys(json_object* jobj, char*** rarr);
#endif