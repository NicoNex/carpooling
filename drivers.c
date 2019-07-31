/*
 * carpooling
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
	refresh_driver_ids(node->next);
}


list_t load_drivers() {
	list_t drivers_list = NULL;
	int drivers_num;
	struct json_object *json = load_json_from_file(DRIVERS_FILE);
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


void update_drivers_file(list_t lst) {
	struct json_object  *main,
						*array,
						*driver,
						*id,
						*name,
						*age,
						*vehicle,
						*seats,
						*rating;

	main = json_object_new_object();
	array = json_object_new_array();

	for (list_t tmp = lst; tmp; tmp = next(tmp)) {
		struct driver *drv = get_object(tmp);

		driver = json_object_new_object();
		id = json_object_new_int(drv->id);
		name = json_object_new_string(drv->name);
		age = json_object_new_int(drv->age);
		vehicle = json_object_new_string(drv->vehicle);
		seats = json_object_new_int(drv->seats);
		rating = json_object_new_int(drv->rating);

		json_object_object_add(driver, "id", id);
		json_object_object_add(driver, "name", name);
		json_object_object_add(driver, "age", age);
		json_object_object_add(driver, "vehicle", vehicle);
		json_object_object_add(driver, "seats", seats);
		json_object_object_add(driver, "rating", rating);

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

	for (list_t tmp = node; tmp; tmp = next(tmp)) {
		struct driver *drv = tmp->ptr;

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


// TODO: maybe make this recursive
void dispose_drivers(const list_t lst) {
	for (list_t tmp = lst; tmp != NULL; tmp = next(tmp))
		free(tmp->ptr);

	dispose_list(lst);
}
