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
 * TODO: doc
 */
list_t new_list(void *ptr);

/*
 * TODO: doc
 */
list_t list_add(list_t list, void *ptr);

/*
 * TODO: doc, broken
 */
// int list_append(list_t list, void *ptr);

/*
 * TODO: doc
 */
list_t list_del(list_t list, struct node *node);

/*
 * Returns the next element in a list
 */
list_t next(list_t current);

/*
 * Returns the object contained in a node
 */
void *get_object(list_t current);

/*
 * TODO: doc
 */
void dispose_list(list_t list);


#endif // LIST_H_
