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

enum modes {
	DEFAULT,
	SELECT_DRIVER,
	RATE
};


struct bot {
	int64_t chat_id;
	int mode;
	list_t drivers;
	struct driver *drvtmp;
};


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	bot->mode = DEFAULT;
	bot->drvtmp = NULL;
	bot->drivers = load_drivers("res/drivers.json");
	return bot;
}


void send_drivers(struct bot *bot) {
	for (list_t tmp = bot->drivers; tmp != NULL; tmp = next(tmp)) {
		struct driver *drv = get_object(tmp);
		char msg[512];

		snprintf(msg, 512, "Name: %s%%0AID: %d%%0AVehicle: %s%%0ASeats: %d%%0A%%0A", drv->name, drv->id, drv->vehicle, drv->seats);
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

		if (!strcmp(text, "/dismiss")) {
			bot->mode = DEFAULT;
			tg_send_message("Action dismissed", bot->chat_id);
			return;
		}

		if (bot->mode == DEFAULT) {
			if (!strcmp(text, "/ping"))
				tg_send_message("Hey!", bot->chat_id);

			else if (!strcmp(text, "/drivers")) {
				bot->drivers = load_drivers("res/drivers.json");
				send_drivers(bot);
			}

			else if (!strcmp(text, "/rate")) {
				bot->mode = SELECT_DRIVER;
				tg_send_message("Scrivimi l'ID del guidatore che vuoi valutare", bot->chat_id);
			}
		}

		else {
			switch (bot->mode) {
				case SELECT_DRIVER: {
					// debug this shit
					int id = strtol(text, NULL, 10);
					bot->drvtmp = get_driver(bot->drivers, id);

					if (bot->drvtmp == NULL) {
						tg_send_message("ID incorretto.%0AScrivi solo il numero dell'ID del guidatore da valutare", bot->chat_id);
						break;
					}

					char msg[512];
					snprintf(msg, 512, "Scrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					bot->mode = RATE;
					break;
				}

				case RATE: {
					int rating = strtol(text, NULL, 10);

					if (rating < 1 || rating > 11) {
						char msg[512];
						snprintf(msg, 512, "Valutazione incorretta.%0AScrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
						tg_send_message(msg, bot->chat_id);
					}

					bot->drvtmp->rating = rating;
					update_driver("res/drivers.json", bot->drvtmp);
					bot->drvtmp = NULL;
					bot->mode = DEFAULT;
					break;
				}

			}
		}
	}
}
