/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "include/list.h"
#include "include/travels.h"
#include "include/filehandler.h"


list_t load_travels() {
	list_t travels_list = NULL;
	int travels_num;
	struct json_object *json = load_json_from_file(TRAVELS_FILE);
	struct json_object *travels_json;

	json_object_object_get_ex(json, "travels", &travels_json);
	travels_num = json_object_array_length(travels_json);

	for (int i = 0; i < travels_num; i++) {
		struct json_object *travel_obj, *driver_id, *destination, *date, *id;

		travel_obj = json_object_array_get_idx(travels_json, i);

		if (!(json_object_object_get_ex(travel_obj, "driver_id", &driver_id)
				&& json_object_object_get_ex(travel_obj, "destination", &destination)
				&& json_object_object_get_ex(travel_obj, "id", &id)
				&& json_object_object_get_ex(travel_obj, "date", &date))) {
			continue;
		}

		struct travel *trv = malloc(sizeof(struct travel));

		trv->id = json_object_get_int(id);
		trv->date = json_object_get_string(date);
		trv->driver_id = json_object_get_int(driver_id);
		trv->destination = json_object_get_string(destination);

		travels_list = list_add(travels_list, trv);
	}

	return travels_list;
}


static void refresh_travel_ids(list_t node) {
	static int counter;

	if (node == NULL) {
		counter = 1;
		return;
	}

	struct travel *tmp = node->ptr;
	tmp->id = counter++;
	refresh_travel_ids(node->next);
}


static void update_travels_file(list_t lst) {
	struct json_object  *id,
						*date,
						*main,
						*array,
						*travel,
						*driver_name,
						*destination;

	main = json_object_new_object();
	array = json_object_new_array();

	for (list_t tmp = lst; tmp; tmp = next(tmp)) {
		struct travel *trv = get_object(tmp);

		travel = json_object_new_object();
		id = json_object_new_int(trv->id);
		date = json_object_new_string(trv->date);
		driver_name = json_object_new_string(trv->driver_name);
		destination = json_object_new_string(trv->destination);

		json_object_object_add(travel, "id", id);
		json_object_object_add(travel, "date", date);
		json_object_object_add(travel, "driver_name", driver_name);
		json_object_object_add(travel, "destination", destination);

		json_object_array_add(array, travel);
	}

	json_object_object_add(main, "travels", array);

	FILE *fp = fopen(TRAVELS_FILE, "w");
	char *text = json_object_to_json_string_ext(main, JSON_C_TO_STRING_PRETTY);
	puts(text); // DEBUG
	fwrite(text, 1, strlen(text), fp);
	fclose(fp);
	json_object_put(main);
}


list_t add_travel(list_t travels, struct travel *trv) {
	list_t first = list_add(travels, trv);
	refresh_travel_ids(first);
	update_travels_file(first);
	return first;
}


list_t del_travel(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	list_t first = node;
	list_t prev = NULL;

	for (list_t tmp = node; node; node = next(node)) {
		struct travel *trv = get_object(node);

		if (trv->id == id) {
			if (prev)
				prev->next = tmp->next;
			else
				first = tmp->next;

			free(trv);
			free(tmp);
			break;
		}
	}

	refresh_travel_ids(first);
	update_travels_file(first);
	return first;
}


struct travel *get_travel(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	struct travel *drv = node->ptr;
	if (drv->id == id)
		return drv;

	return get_travel(node->next, id);
}


void dispose_travels(list_t lst) {
	for (list_t tmp = lst; tmp != NULL; tmp = next(tmp))
		free(tmp->ptr);

	dispose_list(lst);
}
