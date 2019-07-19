/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <stdio.h>
#include <string.h>

#include "utron/bot.h"
#include "utron/engine.h"
#include "utron/dispatcher.h"
#include "include/drivers.h"
#include "include/travels.h"


#define BOTNAME "carpooling"

enum modes {
	DEFAULT,
	SELECT_DRIVER,
	RATE
};


struct bot {
	int64_t chat_id;
	int mode;
	struct driver *drvtmp;
};


volatile list_t drivers;
volatile list_t travels;


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	bot->mode = DEFAULT;
	bot->drvtmp = NULL;

	return bot;
}


void send_drivers(int64_t chat_id) {
	for (list_t tmp = drivers; tmp != NULL; tmp = next(tmp)) {
		struct driver *drv = get_object(tmp);
		char msg[512];

		snprintf(msg, 512, "ID: %d%%0AName: %s%%0AVehicle: %s%%0ASeats: %d%%0ARating: %d%%0A%%0A", drv->id, drv->name, drv->vehicle, drv->seats, drv->rating);
		tg_send_message(msg, chat_id);
	}

}


void send_travels(int64_t chat_id) {
	for (list_t tmp = travels; tmp != NULL; tmp = next(tmp)) {
		struct travel *trv = get_object(tmp);
		struct driver *drv = get_driver(drivers, trv->driver_id);
		char *nametmp;
		char msg[512];

		if (trv == NULL) return;
		(drv) ? (nametmp = drv->name) : (nametmp = "N/A");
		snprintf(msg, 512, "Destination: %s%%0ADate: %s%%0ADriver: %s%%0A%%0A", trv->destination, trv->date, nametmp);
		tg_send_message(msg, chat_id);
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

		// we need to check for "/dismiss" in any case
		if (!strcmp(text, "/dismiss")) {
			bot->mode = DEFAULT;
			tg_send_message("Action dismissed", bot->chat_id);
			return NULL;
		}

		// evaluates whether in DEFAULT mode in other to skip evaluation of otherwhise useless commands
		if (bot->mode == DEFAULT) {
			if (!strcmp(text, "/ping"))
				tg_send_message("Hey!", bot->chat_id);

			else if (!strcmp(text, "/drivers")) {
				send_drivers(bot->chat_id);
			}

			else if (!strcmp(text, "/rate")) {
				bot->mode = SELECT_DRIVER;
				tg_send_message("Scrivimi l'ID del guidatore che vuoi valutare", bot->chat_id);
			}

			else if (!strcmp(text, "/travels")) {
				send_travels(bot->chat_id);
			}
		}

		else {
			switch (bot->mode) {
			case SELECT_DRIVER: {
				int id = strtol(text, NULL, 10);
				bot->drvtmp = get_driver(drivers, id);

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
					snprintf(msg, 512, "Valutazione incorretta.%%0AScrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					break;
				}

				bot->drvtmp->rating = rating;
				update_driver("res/drivers.json", bot->drvtmp);
				bot->drvtmp = NULL;
				bot->mode = DEFAULT;
				tg_send_message("Grazie per il tuo feedback!", bot->chat_id);
				break;
			}}
		}
	}
}


int main(void) {
	drivers = load_drivers("res/drivers.json");
	travels = load_travels("res/travels.json");
	run_dispatcher("568059758:AAFRN3Xg3dOkfe2n0gNlOWjlkM6dihommPQ");
}
