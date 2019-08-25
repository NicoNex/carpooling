/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <stdio.h>
#include <stdlib.h>

#include "include/list.h"


list_t new_list(void *ptr) {
	struct node *node = malloc(sizeof(node));

	if (node == NULL) {
		fputs("error: not enough memory, malloc returned NULL\n", stderr);
		return NULL;
	}

	node->ptr = ptr;
	node->next = NULL;
	return node;
}


list_t list_add(struct node *first, void *ptr) {
	struct node *node = malloc(sizeof(node));

	if (node == NULL) {
		fputs("error: not enough memory, malloc returned NULL\n", stderr);
		return first;
	}

	node->ptr = ptr;
	node->next = first;
	return node;
}


list_t list_del(struct node *first, struct node *delptr) {
	struct node *prev = NULL;
	struct node *current = first;

	while (current != NULL) {
		if (current == delptr) {
			if (prev == NULL) {
				struct node *tmp = current->next;
				free(current);
				return tmp;
			}


			prev->next = current->next;
			free(current);
			break;
		}

		prev = current;
		current = current->next;
	}

	return first;
}


// void *range(static struct node *node) {
// 	void *ptr;

// 	if (current == NULL)
// 		return NULL;

// 	ptr = current->ptr;
// 	current = current->next;
// 	return ptr;
// }


struct node *next(struct node *current) {
	return current->next;
}


void *get_object(struct node *current) {
	return current->ptr;
}


void dispose_list(struct node *node) {
	if (node == NULL)
		return;

	struct node *next = node->next;
	free(node);
	dispose_list(next);
}
