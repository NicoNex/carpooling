/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/drivers.h"
#include "include/filehandler.h"


static void refresh_driver_ids(list_t node) {
	static int counter = 1;

	if (node == NULL) {
		counter = 1;
		return;
	}

	struct driver *tmp = node->ptr;
	tmp->id = counter++;
	refresh_driver_ids(NEXT(node));
}


list_t load_drivers() {
	list_t drivers_list = NULL;
	int drivers_num;
	struct json_object *json = load_json_from_file(DRIVERS_FILE);
	struct json_object *drivers_json;

	json_object_object_get_ex(json, "drivers", &drivers_json);
	drivers_num = json_object_array_length(drivers_json);

	for (int i = 0; i < drivers_num; i++) {
		struct json_object  *id,
							*age,
							*name,
							*token,
							*rating,
							*vehicle,
							*driverobj;

		driverobj = json_object_array_get_idx(drivers_json, i);

		if (!(json_object_object_get_ex(driverobj, "id", &id)
				&& json_object_object_get_ex(driverobj, "name", &name)
				&& json_object_object_get_ex(driverobj, "age", &age)
				&& json_object_object_get_ex(driverobj, "vehicle", &vehicle)
				&& json_object_object_get_ex(driverobj, "rating", &rating)
				&& json_object_object_get_ex(driverobj, "token", &token))) {
			continue;
		}

		struct driver *drv = malloc(sizeof(struct driver));
		drv->id = json_object_get_int(id);
		drv->age = json_object_get_int(age);
		drv->name = json_object_get_string(name);
		drv->token = json_object_get_int64(token);
		drv->rating = json_object_get_int(rating);
		drv->vehicle = json_object_get_string(vehicle);

		drivers_list = list_add(drivers_list, drv);
	}

	return drivers_list;
}


struct driver *get_driver_by_id(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	struct driver *drv = GET_OBJ(node);
	if (drv->id == id)
		return drv;

	return get_driver_by_id(NEXT(node), id);

}


struct driver *get_driver_by_token(list_t node, const int64_t token) {
	if (node == NULL)
		return NULL;

	struct driver *drv = GET_OBJ(node);
	if (drv->token == token)
		return drv;

	return get_driver_by_token(NEXT(node), token);
}


void update_drivers_file(list_t lst) {
	struct json_object  *main,
						*array,
						*driver,
						*id,
						*name,
						*age,
						*vehicle,
						*token,
						*rating;

	main = json_object_new_object();
	array = json_object_new_array();

	for (list_t tmp = lst; tmp; tmp = NEXT(tmp)) {
		struct driver *drv = GET_OBJ(tmp);

		driver = json_object_new_object();
		id = json_object_new_int(drv->id);
		name = json_object_new_string(drv->name);
		age = json_object_new_int(drv->age);
		vehicle = json_object_new_string(drv->vehicle);
		rating = json_object_new_int(drv->rating);
		token = json_object_new_int64(drv->token);

		json_object_object_add(driver, "id", id);
		json_object_object_add(driver, "name", name);
		json_object_object_add(driver, "age", age);
		json_object_object_add(driver, "vehicle", vehicle);
		json_object_object_add(driver, "rating", rating);
		json_object_object_add(driver, "token", token);

		json_object_array_add(array, driver);
	}

	json_object_object_add(main, "drivers", array);

	FILE *fp = fopen(DRIVERS_FILE, "w");
	char *text = json_object_to_json_string_ext(main, JSON_C_TO_STRING_PRETTY);
	fwrite(text, 1, strlen(text), fp);
	fclose(fp);
	json_object_put(main);
}


list_t add_driver(list_t drivers, struct driver *drv) {
	list_t first = list_add(drivers, drv);
	refresh_driver_ids(first);
	update_drivers_file(first);
	return first;
}


list_t del_driver(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	list_t first = node;
	list_t prev = NULL;

	for (list_t tmp = node; tmp; tmp = NEXT(tmp)) {
		struct driver *drv = GET_OBJ(tmp);

		if (drv->id == id) {
			if (prev)
				prev->next = tmp->next;
			else
				first = tmp->next;

			free(drv);
			free(tmp);
			break;
		}

		prev = tmp;
	}

	refresh_driver_ids(first);
	update_drivers_file(first);
	return first;
}


void dispose_drivers(const list_t lst) {
	for (list_t tmp = lst; tmp != NULL; tmp = NEXT(tmp))
		free(tmp->ptr);

	dispose_list(lst);
}
