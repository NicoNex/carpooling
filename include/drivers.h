/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "list.h"


#define DRIVERS_FILE "res/drivers.json"


struct driver {
	int id;
	int age;
	// int seats;
	int rating;
	char *name;
	char *vehicle;
	int64_t token;
};

// Loads the drivers list from disk from a json file.
list_t load_drivers();

// Frees the memory occupied by the drivers list
void dispose_drivers(const list_t drivers);

// Updates the file that stores the information regarding the drivers
void update_drivers_file(const list_t drivers);


// Adds a new driver to the list
list_t add_driver(list_t drivers, struct driver *drv);

// Deletes the driver that has the specified ID
list_t del_driver(list_t drivers, const int id);

// Returns the pointer to the driver with the specified id
struct driver *get_driver_by_id(list_t drivers, const int id);


// Returns the pointer to the driver with the specified token.
struct driver *get_driver_by_token(list_t drivers, const int64_t token);


#endif // DRIVERS_H_
