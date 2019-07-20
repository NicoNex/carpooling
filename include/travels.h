/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef TRAVELS_H_
#define TRAVELS_H_


#include <stdint.h>
#include "list.h"


struct travel {
	int id;
	int driver_id;
	int64_t unix_time;
	char *destination;
	char *date;
};

// Loads the drivers list from disk from a json file.
list_t load_travels(const char *filepath);

// Updates the driver on the file
void update_travel(const char *filepath, struct travel *trv);

// returns the travel pointer in a list with the corresponding id
struct travel *get_travel(list_t travels, const int id);


// Frees the memory occupied by the drivers list
void dispose_travels(const list_t travels);



#endif // TRAVELS_H_
