/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/drivers.h"


list_t drivers_list = NULL;


void load_drivers(const char *filepath) {
	FILE *fp;
	char *buffer;
	long file_size;
	int drivers_num;
	struct json_object *json;
	struct json_object *drivers_json;

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "error: cannot open file %s", filepath);
		return;
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
}
