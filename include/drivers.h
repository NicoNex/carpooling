/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef DRIVERS_H_
#define DRIVERS_H_

#include "list.h"


struct driver {
	int id;
	int age;
	int seats;
	int rating;
	char *name;
	char *vehicle;
};

// Loads the drivers list from disk from a json file.
list_t load_drivers(const char *filepath);

// Frees the memory occupied by the drivers list
void dispose_drivers(const list_t drivers);

// Updates the driver
// void update_driver(const list_t drivers, const struct driver *drv);

struct driver *get_driver(list_t drivers, const int id);


#endif // DRIVERS_H_
