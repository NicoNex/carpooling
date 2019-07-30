/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef TRAVELS_H_
#define TRAVELS_H_


#include <time.h>
#include <stdint.h>
#include "list.h"


#define TRAVELS_FILE "res/travels.json"


struct travel {
	int id;
	char *date;
	int driver_id; // deprecated
	time_t epoch;
	char *destination;
	char *driver_name;
};

// Loads the travels list from disk from a json file.
list_t load_travels();


// returns the travel pointer in a list with the corresponding id
struct travel *get_travel(list_t travels, const int id);


// adds a travel to the travels list
list_t add_travel(list_t travels, struct travel *travel);

// deletes a travel from the travels list
list_t del_travel(list_t travels, const int id);

// Updates the file that stores the information regarding the travels
void update_travels_file(const list_t travels);


// Frees the memory occupied by the travels list
void dispose_travels(const list_t travels);



#endif // TRAVELS_H_
