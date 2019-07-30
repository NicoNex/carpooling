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
	int seats;
	int rating;
	char *name;
	char *vehicle;
};

// Loads the drivers list from disk from a json file.
list_t load_drivers();

// Frees the memory occupied by the drivers list
void dispose_drivers(const list_t drivers);

// Updates the file that stores the information regarding the drivers
void update_drivers_file(const list_t drivers);

// Updates the driver
void update_driver(struct driver *drv);


// Used to add a new driver to the list
list_t add_driver(list_t drivers, struct driver *drv);


list_t del_driver(list_t drivers, const int id);

struct driver *get_driver(list_t drivers, const int id);


#endif // DRIVERS_H_
