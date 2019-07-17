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


void load_drivers(const char *filepath);


#endif // DRIVERS_H_
