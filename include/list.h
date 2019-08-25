/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef LIST_H_
#define LIST_H_


struct node {
	void *ptr;
	struct node *next;
};


typedef struct node *list_t;

// Macro to get the pointer to the object contained in the node.
#define GET_OBJ(node) (node->ptr)

// Macro to get the pointer to the next node in the list.
#define NEXT(node) (node->next)

// Returns the pointer to the head of the list.
list_t new_list(void *ptr);

// Adds an element to the list and returns the pointer to the head.
list_t list_add(list_t list, void *ptr);

// Deletes a node from the list and returns the head.
list_t list_del(list_t list, struct node *node);

// Frees the memory occupied by the list.
void dispose_list(list_t list);


#endif // LIST_H_
