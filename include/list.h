/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef LIST_H_
#define LIST_H_


struct node {
	void *ptr;
	struct node *next;
};


typedef struct node *list_t;


#define GET_OBJ(node) (node->ptr)
#define NEXT(node) (node->next)

/*
 * Returns the pointer to the first node of a list.
 */
list_t new_list(void *ptr);

/*
 * Adds an element to the list and returns the pointer to the head.
 */
list_t list_add(list_t list, void *ptr);

/*
 * Deletes a node from the list and returns the pointer to the head.
 */
list_t list_del(list_t list, struct node *node);

/*
 * This function frees the list.
 */
void dispose_list(list_t list);


#endif // LIST_H_
