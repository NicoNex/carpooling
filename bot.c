/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <string.h>

#include "utron/bot.h"
#include "utron/engine.h"
#include "include/drivers.h"


#define BOTNAME "carpooling"


struct bot {
	int64_t chat_id;
};


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	return bot;
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

		if (!strcmp(text, "/ping")) {
			tg_send_message("Hey!", bot->chat_id);
			load_drivers("res/drivers.json");
		}
	}
}
