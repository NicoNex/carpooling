/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <stdio.h>
#include <string.h>

#include "utron/bot.h"
#include "utron/engine.h"
#include "include/drivers.h"


#define BOTNAME "carpooling"


struct bot {
	int64_t chat_id;
	list_t drivers;
};


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	bot->drivers = load_drivers("res/drivers.json");
	return bot;
}


void send_drivers(struct bot *bot) {
	for (list_t tmp = bot->drivers; tmp != NULL; tmp = next(tmp)) {
		struct driver *drv = get_object(tmp);
		char msg[512];

		snprintf(msg, 512, "Name: %s%%0AVehicle: %s%%0ASeats: %d%%0A%%0A", drv->name, drv->vehicle, drv->seats);
		tg_send_message(msg, bot->chat_id);
	}

}


void *update_bot(void *vargp) {
	struct bot_update_arg *arg = vargp;
	struct bot *bot = arg->bot_ptr;
	struct json_object *update = arg->update;
	struct json_object *messageobj, *fromobj, *usrobj, *textobj;

	if (json_object_object_get_ex(update, "message", &messageobj)
			&& json_object_object_get_ex(messageobj, "from", &fromobj)
			&& json_object_object_get_ex(messageobj, "text", &textobj)
			&& json_object_object_get_ex(fromobj, "username", &usrobj)) {


		const char *text = json_object_get_string(textobj);
		const char *username = json_object_get_string(usrobj);

		if (!strcmp(text, "/ping"))
			tg_send_message("Hey!", bot->chat_id);

		else if (!strcmp(text, "/drivers"))
			send_drivers(bot);
	}
}
