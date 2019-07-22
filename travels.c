/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */


#include <stdint.h>
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
