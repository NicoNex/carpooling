/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef LIST_H_
#define LIST_H_


struct node {
	void *objptr;
	struct node *next;
};


typedef struct node *list_t;

/*
 * TODO: doc
 */
list_t new_list(void *objptr);

/*
 * TODO: doc
 */
list_t list_add(list_t list, void *objptr);

/*
 * TODO: doc
 */
list_t list_del(list_t list, struct node *node);

/*
 * TODO: doc
 */
void dispose_list(list_t list);


#endif // LIST_H_
