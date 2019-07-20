/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#include <ctype.h>
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
	RATE,
	ADD_NAME,
	ADD_AGE,
	ADD_VEHICLE,
	ADD_SEATS,
	CONFIRM_ADD,
	CONFIRM_UPD
};


struct bot {
	int64_t chat_id;
	int mode;
	int next_mode;
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

		snprintf(msg, 512, "ID: %d%%0ANome: %s%%0AEtà: %d%%0AVeicolo: %s%%0APosti: %d%%0AValutazione: %d%%0A%%0A", drv->id, drv->name, drv->age, drv->vehicle, drv->seats, drv->rating);
		tg_send_message(msg, chat_id);
	}

}


void send_travels(int64_t chat_id) {
	for (list_t tmp = travels; tmp != NULL; tmp = next(tmp)) {
		struct travel *trv = get_object(tmp);
		struct driver *drv = get_driver(drivers, trv->driver_id);
		char *nametmp;
		char msg[512];

		// let's avoid SEGFAULT like a pro
		if (trv == NULL) continue;
		(drv) ? (nametmp = drv->name) : (nametmp = "N/A");

		snprintf(msg, 512, "ID: %d%%0ADestination: %s%%0ADate: %s%%0ADriver: %s%%0A%%0A", trv->id, trv->destination, trv->date, nametmp);
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
		if (!strcmp(text, "/annulla")) {
			bot->mode = DEFAULT;
			tg_send_message("Action dismissed", bot->chat_id);
			return NULL;
		}

		// evaluates whether in DEFAULT mode in other to skip evaluation of otherwhise useless commands
		if (bot->mode == DEFAULT) {
			if (!strcmp(text, "/ping"))
				tg_send_message("Hey!", bot->chat_id);

			else if (!strcmp(text, "/guidatori"))
				send_drivers(bot->chat_id);

			else if (!strcmp(text, "/valuta")) {
				bot->mode = SELECT_DRIVER;
				bot->next_mode = RATE;
				tg_send_message("Scrivimi l'ID del guidatore che vuoi valutare", bot->chat_id);
			}

			else if (!strcmp(text, "/viaggi"))
				send_travels(bot->chat_id);

			else if (!strcmp(text, "/agg_guidatore")) {
				bot->mode = ADD_NAME;
				bot->drvtmp = calloc(1, sizeof(struct driver));
				tg_send_message("Scrivimi il nome del guidatore che vuoi aggiungere", bot->chat_id);
			}

			else if (!strcmp(text, "/mod_guidatore")) {
				bot->mode = SELECT_DRIVER;
				bot->next_mode = ADD_NAME;
				tg_send_message("Scrivimi l'ID del guidatore che vuoi modificare", bot->chat_id);
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

				char msg[512] = {'\0'};

				switch (bot->next_mode) {
				case RATE:
					snprintf(msg, 512, "Scrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					break;

				case ADD_NAME:
					snprintf(msg, 512, "Scrivi il nuovo nome di %s", bot->drvtmp->name);
					break;
				}

				tg_send_message(msg, bot->chat_id);
				bot->mode = bot->next_mode;
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
				update_driver(bot->drvtmp);
				bot->drvtmp = NULL;
				bot->mode = DEFAULT;
				tg_send_message("Grazie per il tuo feedback!", bot->chat_id);
				break;
			}

			case ADD_NAME:
				bot->drvtmp->name = text;
				bot->mode = ADD_AGE;
				tg_send_message("Grazie, adesso inviami l'età del guidatore scrivendo solo il numero degli anni", bot->chat_id);
				break;

			case ADD_AGE: {
				char msg[512];
				int age = strtol(text, NULL, 10);

				if (!age) {
					snprintf(msg, 512, "Età incorretta.%%0AScrivi l'età di %s mandando solo il numero degli anni", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					break;
				}

				bot->drvtmp->age = age;
				bot->mode = ADD_VEHICLE;
				snprintf(msg, 512, "Grazie, adesso inviami il tipo di veicolo guidato da %s", bot->drvtmp->name);
				tg_send_message(msg, bot->chat_id);
				break;
			}

			case ADD_VEHICLE:
				bot->drvtmp->vehicle = text;
				bot->mode = ADD_SEATS;
				tg_send_message("Grazie, inviami il numero di posti dispobibili", bot->chat_id);
				break;


			case ADD_SEATS: {
				char msg[512];
				int seats = strtol(text, NULL, 10);

				if (seats < 1) {
					snprintf(msg, 512, "Numero posti non valido, immetti il numero di posti disponibili per %s", bot->drvtmp->name);
					break;
				}

				bot->drvtmp->seats = seats;
				bot->mode = CONFIRM_ADD;
				snprintf(msg, 512, "Nome: %s%%0AEtà: %d%%0AVeicolo: %s%%0APosti: %d", bot->drvtmp->name, bot->drvtmp->age, bot->drvtmp->vehicle, bot->drvtmp->seats);
				tg_send_message(msg, bot->chat_id);
				tg_send_message("Confermi? [S/N]", bot->chat_id);
				break;
			}


			case CONFIRM_ADD: {
				char response = tolower(text[0]);

				if (response == 's') {
					drivers = add_driver(drivers, bot->drvtmp);
					bot->mode = DEFAULT;
					send_drivers(bot->chat_id);
				}

				else if (response == 'n') {
					tg_send_message("Inserimento annullato", bot->chat_id);
					bot->mode = DEFAULT;
				}

				else
					tg_send_message("Risposta non valida, scrivi 's' per confermare o 'n' per annullare", bot->chat_id);

				break;
			}

			case CONFIRM_UPD: {
				char response = tolower(text[0]);

				if (response == 's') {
					update_driver(bot->drvtmp);
					bot->mode = DEFAULT;
					send_drivers(bot->chat_id);
				}

				else if (response == 'n') {
					tg_send_message("Inserimento annullato", bot->chat_id);
					bot->mode = DEFAULT;
				}

				else
					tg_send_message("Risposta non valida, scrivi 's' per confermare o 'n' per annullare", bot->chat_id);

				break;
			}}
		}
	}
}


int main(void) {
	drivers = load_drivers();
	travels = load_travels("res/travels.json");
	run_dispatcher("921731218:AAHG29_KgVw-nsEweUmCMTh7fS71u0W6gmw");
}
