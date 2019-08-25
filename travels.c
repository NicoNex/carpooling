/*
 * Kowalski
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
		struct json_object  *id,
							*date,
							*token,
							*price,
							*seats,
							*travel_obj,
							*destination;

		travel_obj = json_object_array_get_idx(travels_json, i);

		if (!(json_object_object_get_ex(travel_obj, "destination", &destination)
				&& json_object_object_get_ex(travel_obj, "id", &id)
				&& json_object_object_get_ex(travel_obj, "date", &date)
				&& json_object_object_get_ex(travel_obj, "price", &price)
				&& json_object_object_get_ex(travel_obj, "seats", &seats)
				&& json_object_object_get_ex(travel_obj, "token", &token))) {
			continue;
		}

		struct travel *trv = malloc(sizeof(struct travel));

		trv->id = json_object_get_int(id);
		trv->date = json_object_get_string(date);
		trv->price = (float) json_object_get_double(price);
		trv->seats = json_object_get_int(seats);
		trv->token = json_object_get_int64(token);
		trv->destination = json_object_get_string(destination);

		travels_list = list_add(travels_list, trv);
	}

	return travels_list;
}


static void refresh_travel_ids(list_t node) {
	static int counter = 1;

	if (node == NULL) {
		counter = 1;
		return;
	}

	struct travel *tmp = GET_OBJ(node);
	tmp->id = counter++;
	refresh_travel_ids(NEXT(node));
}


void update_travels_file(const list_t lst) {
	struct json_object  *id,
						*date,
						*main,
						*array,
						*price,
						*token,
						*seats,
						*travel,
						*destination;

	main = json_object_new_object();
	array = json_object_new_array();

	for (list_t tmp = lst; tmp; tmp = NEXT(tmp)) {
		struct travel *trv = GET_OBJ(tmp);

		travel = json_object_new_object();
		id = json_object_new_int(trv->id);
		date = json_object_new_string(trv->date);
		price = json_object_new_double(trv->price);
		token = json_object_new_int64(trv->token);
		seats = json_object_new_int(trv->seats);
		destination = json_object_new_string(trv->destination);

		json_object_object_add(travel, "id", id);
		json_object_object_add(travel, "date", date);
		json_object_object_add(travel, "price", price);
		json_object_object_add(travel, "token", token);
		json_object_object_add(travel, "seats", seats);
		json_object_object_add(travel, "destination", destination);

		json_object_array_add(array, travel);
	}

	json_object_object_add(main, "travels", array);

	FILE *fp = fopen(TRAVELS_FILE, "w");
	char *text = json_object_to_json_string_ext(main, JSON_C_TO_STRING_PRETTY);
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

	for (list_t tmp = node; tmp; tmp = NEXT(tmp)) {
		struct travel *trv = GET_OBJ(tmp);

		if (trv->id == id) {
			(prev) ? (prev->next = tmp->next) : (first = tmp->next);

			free(trv);
			free(tmp);
			break;
		}
		prev = tmp;
	}

	refresh_travel_ids(first);
	update_travels_file(first);
	return first;
}


list_t del_travels_with_token(list_t node, const int64_t token) {
	if (node == NULL)
		return NULL;

	list_t first = node;
	list_t prev = NULL;
	list_t tmp = node;

	while (tmp) {
		struct travel *trv = GET_OBJ(tmp);

		if (trv->token == token) {
			(prev) ? (prev->next = tmp->next) : (first = tmp->next);

			list_t ntmp = NEXT(tmp);
			free(trv);
			free(tmp);
			tmp = ntmp;
		}

		else {
			prev = tmp;
			tmp = NEXT(tmp);
		}
	}

	refresh_travel_ids(first);
	update_travels_file(first);
	return first;
}


struct travel *get_travel(list_t node, const int id) {
	if (node == NULL)
		return NULL;

	struct travel *trv = GET_OBJ(node);
	if (trv->id == id)
		return trv;

	return get_travel(NEXT(node), id);
}


struct travel *get_travel_with_token(list_t node, const int64_t token) {
	if (node == NULL)
		return NULL;

	struct travel *trv = GET_OBJ(node);
	if (trv->token == token)
		return trv;

	return get_travel_with_token(NEXT(node), token);
}


void dispose_travels(list_t lst) {
	for (list_t tmp = lst; tmp != NULL; tmp = NEXT(tmp))
		free(tmp->ptr);

	dispose_list(lst);
}
