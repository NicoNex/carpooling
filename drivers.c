/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/drivers.h"


static json_object *get_json_from_file(const char *filepath) {
	FILE *fp;
	char *buffer;
	long file_size;
	struct json_object *json;

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "error: cannot open file %s", filepath);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	if (file_size > 0) {
		buffer = malloc(file_size);
		fread(buffer, file_size, 1, fp);
		json = json_tokener_parse(buffer);
		free(buffer);
	}
	fclose(fp);

	return json;
}


list_t load_drivers(const char *filepath) {
	list_t drivers_list = NULL;
	int drivers_num;
	struct json_object *json = get_json_from_file(filepath);
	struct json_object *drivers_json;

	json_object_object_get_ex(json, "drivers", &drivers_json);
	drivers_num = json_object_array_length(drivers_json);
	for (int i = 0; i < drivers_num; i++) {
		struct json_object *driverobj, *id, *name, *age, *vehicle, *rating, *seats;

		driverobj = json_object_array_get_idx(drivers_json, i);

		if (!(json_object_object_get_ex(driverobj, "id", &id)
				&& json_object_object_get_ex(driverobj, "name", &name)
				&& json_object_object_get_ex(driverobj, "age", &age)
				&& json_object_object_get_ex(driverobj, "vehicle", &vehicle)
				&& json_object_object_get_ex(driverobj, "rating", &rating)
				&& json_object_object_get_ex(driverobj, "seats", &seats))) {
			continue;
		}

		struct driver *drv = malloc(sizeof(struct driver));
		drv->id = json_object_get_int(id);
		drv->age = json_object_get_int(age);
		drv->name = json_object_get_string(name);
		drv->seats = json_object_get_int(seats);
		drv->rating = json_object_get_int(rating);
		drv->vehicle = json_object_get_string(vehicle);

		drivers_list = list_add(drivers_list, drv);
	}

	return drivers_list;
}


struct driver *get_driver(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	struct driver *drv = node->ptr;
	if (drv->id == id)
		return drv;

	return get_driver(node->next, id);

}


void update_driver(const char *filepath, struct driver *drv) {
	struct json_object *json = get_json_from_file(filepath);
	struct json_object *drivers_json;
	int drivers_num;

	json_object_object_get_ex(json, "drivers", &drivers_json);
	drivers_num = json_object_array_length(drivers_json);
	for (int i = 0; i < drivers_num; i++) {
		struct json_object *driverobj, *id, *rating, *seats;

		driverobj = json_object_array_get_idx(drivers_json, i);
		json_object_object_get_ex(driverobj, "id", &id);

		if (drv->id == json_object_get_int(id)) {
			json_object_object_get_ex(driverobj, "rating", &rating);
			json_object_object_get_ex(driverobj, "seats", &seats);

			json_object_set_int(rating, drv->rating);
			json_object_set_int(seats, drv->seats);
			break;
		}
	}

	// write the json to the file
	char *text = json_object_to_json_string(json);
	FILE *fp;
	fp = fopen(filepath, "w");
	fwrite(text, 1, strlen(text), fp);
	fclose(fp);
	free(text);
}


// TODO: maybe make this recursive
void dispose_drivers(const list_t lst) {
	for (list_t tmp = lst; tmp != NULL; tmp = next(tmp))
		free(tmp->ptr);

	dispose_list(lst);
}
