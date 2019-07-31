/*
 * carpooling
 * Copyright (C) 2019  Nicolò Santamaria
 */

#define _XOPEN_SOURCE
#include <time.h>
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
	CONFIRM_ADD_DRV,
	CONFIRM_UPD_DRV,
	DEL_DRIVER,
	ADD_DESTINATION,
	ADD_DATE,
	ADD_DRIVER_ID,
	MOD_TRAVEL,
	CONFIRM_ADD_TRV,
	CONFIRM_UPD_TRV,
	DEL_TRAVEL
};


struct bot {
	int64_t chat_id;
	int mode;
	int next_mode;
	struct driver *drvtmp;
	struct travel *trvtmp;
};

// volatile for better thread syncronization
volatile list_t drivers;
volatile list_t travels;


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	bot->mode = DEFAULT;
	bot->drvtmp = NULL;

	return bot;
}


static void send_drivers(int64_t chat_id) {
	for (list_t tmp = drivers; tmp != NULL; tmp = next(tmp)) {
		struct driver *drv = get_object(tmp);
		char msg[512];

		snprintf(msg, 512, "ID: %d%%0ANome: %s%%0AEtà: %d%%0AVeicolo: %s%%0APosti: %d%%0AValutazione: %d%%0A%%0A", drv->id, drv->name, drv->age, drv->vehicle, drv->seats, drv->rating);
		tg_send_message(msg, chat_id);
	}

}


static void send_travels(int64_t chat_id) {
	for (list_t tmp = travels; tmp != NULL; tmp = next(tmp)) {
		struct travel *trv = get_object(tmp);
		char msg[511];

		snprintf(msg, 511, "ID: %d%%0ADestinazione: %s%%0AData: %s%%0AGuidatore: %s%%0A%%0A", trv->id, trv->destination, trv->date, trv->driver_name);
		tg_send_message(msg, chat_id);
	}
}


void *update_bot(struct bot *bot, struct json_object *update) {
	struct json_object *messageobj, *fromobj, *usrobj, *textobj;

	if (json_object_object_get_ex(update, "message", &messageobj)
			&& json_object_object_get_ex(messageobj, "from", &fromobj)
			&& json_object_object_get_ex(messageobj, "text", &textobj)
			&& json_object_object_get_ex(fromobj, "username", &usrobj)) {


		const char *text = json_object_get_string(textobj);
		const char *username = json_object_get_string(usrobj);

		// TODO: make it free trvtmp and drvtmp otherwise it's hamarah
		// we need to check for "/annulla" in any case
		if (!strcmp(text, "/annulla")) {
			bot->mode = DEFAULT;
			tg_send_message("Azione annullata", bot->chat_id);
			return NULL;
		}

		switch (bot->mode) {
			case DEFAULT:
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

				else if (!strcmp(text, "/canc_guidatore")) {
					bot->mode = DEL_DRIVER;
					tg_send_message("Scrivimi l'ID del guidatore che vuoi cancellare", bot->chat_id);
				}

				else if (!strcmp(text, "/agg_viaggio")) {
					bot->mode = ADD_DESTINATION;
					bot->next_mode = CONFIRM_ADD_TRV;
					bot->trvtmp = calloc(1, sizeof(struct travel));
					tg_send_message("Scrivimi la destinazione del nuovo viaggio", bot->chat_id);
				}

				// TODO: complete this command
				else if (!strcmp(text, "/mod_viaggio")) {
					bot->mode = MOD_TRAVEL;
					bot->next_mode = CONFIRM_UPD_TRV;
					tg_send_message("Scrivimi l'ID del viaggio che vuoi modificare", bot->chat_id);
				}

				else if (!strcmp(text, "/canc_viaggio")) {
					bot->mode = DEL_TRAVEL;
					tg_send_message("Scrivimi l'ID del viaggio che vuoi cancellare", bot->chat_id);
				}

				break;

			case SELECT_DRIVER: {
				int id = strtol(text, NULL, 10);
				bot->drvtmp = get_driver(drivers, id);

				if (bot->drvtmp == NULL) {
					tg_send_message("ID incorretto.%0AScrivi solo il numero dell'ID del guidatore da valutare", bot->chat_id);
					break;
				}

				char msg[511] = {'\0'};

				switch (bot->next_mode) {
				case RATE:
					snprintf(msg, 511, "Scrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					break;

				case ADD_NAME:
					snprintf(msg, 511, "Scrivi il nuovo nome di %s", bot->drvtmp->name);
					break;
				}

				tg_send_message(msg, bot->chat_id);
				bot->mode = bot->next_mode;
				break;
			}

			case RATE: {
				int rating = strtol(text, NULL, 10);

				if (rating < 1 || rating > 10) {
					char msg[512];
					snprintf(msg, 512, "Valutazione incorretta.%%0AScrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					break;
				}

				bot->drvtmp->rating = rating;
				update_drivers_file(drivers);
				bot->drvtmp = NULL;
				bot->mode = DEFAULT;
				tg_send_message("Grazie per il tuo feedback!", bot->chat_id);
				break;
			}

			case ADD_NAME: {
				size_t len = strlen(text) + 1;
				bot->drvtmp->name = malloc(len);
				strncpy(bot->drvtmp->name, text, len);
				bot->mode = ADD_AGE;
				tg_send_message("Inviami l'età del guidatore scrivendo solo il numero degli anni", bot->chat_id);
				break;
			}

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
				snprintf(msg, 512, "Inviami il tipo di veicolo guidato da %s", bot->drvtmp->name);
				tg_send_message(msg, bot->chat_id);
				break;
			}

			case ADD_VEHICLE: {
				size_t len = strlen(text) + 1;
				bot->drvtmp->vehicle = malloc(len);
				strncpy(bot->drvtmp->vehicle, text, len);
				bot->mode = ADD_SEATS;
				tg_send_message("Inviami il numero di posti dispobibili", bot->chat_id);
				break;
			}

			case ADD_SEATS: {
				char msg[511];
				int seats = strtol(text, NULL, 10);

				if (seats < 1) {
					snprintf(msg, 511, "Numero posti non valido, immetti il numero di posti disponibili per %s", bot->drvtmp->name);
					break;
				}

				bot->drvtmp->seats = seats;
				bot->mode = CONFIRM_ADD_DRV;
				snprintf(msg, 511, "Nome: %s%%0AEtà: %d%%0AVeicolo: %s%%0APosti: %d", bot->drvtmp->name, bot->drvtmp->age, bot->drvtmp->vehicle, bot->drvtmp->seats);
				puts(msg);
				tg_send_message(msg, bot->chat_id);
				tg_send_message("Confermi? [S/N]", bot->chat_id);
				break;
			}

			case CONFIRM_ADD_DRV: {
				char response = tolower(text[0]);

				if (response == 's') {
					drivers = add_driver(drivers, bot->drvtmp);
					bot->mode = DEFAULT;
					send_drivers(bot->chat_id);
				}

				else if (response == 'n') {
					bot->mode = DEFAULT;
					free(bot->drvtmp);
					tg_send_message("Inserimento annullato", bot->chat_id);
				}

				else
					tg_send_message("Risposta non valida, scrivi 's' per confermare o 'n' per annullare", bot->chat_id);

				break;
			}

			case CONFIRM_UPD_DRV: {
				char response = tolower(text[0]);

				if (response == 's') {
					update_drivers_file(drivers);
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

			case DEL_DRIVER: {
				int id = strtol(text, NULL, 10);
				if (!get_driver(drivers, id)) {
					tg_send_message("ID non valido%%0AInviami un ID valido", bot->chat_id);
					break;
				}

				drivers = del_driver(drivers, id);
				tg_send_message("Guidatore cancellato", bot->chat_id);
				bot->mode = DEFAULT;
				break;
			}

			case DEL_TRAVEL: {
				int id = strtol(text, NULL, 10);
				if (!get_travel(travels, id)) {
					tg_send_message("ID non valido%%0AInviami un ID valido", bot->chat_id);
					break;
				}

				travels = del_travel(travels, id);
				tg_send_message("Viaggio cancellato", bot->chat_id);
				bot->mode = DEFAULT;
				break;
			}

			case MOD_TRAVEL: {
				int id = strtol(text, NULL, 10);
				bot->trvtmp = get_travel(travels, id);

				if (bot->trvtmp == NULL) {
					tg_send_message("ID incorretto.%0AScrivi solo il numero dell'ID del viaggio da modificare", bot->chat_id);
					break;
				}

				bot->mode = ADD_DESTINATION;
				tg_send_message("Scrivimi la destinazione del nuovo viaggio", bot->chat_id);
				break;
			}

			case ADD_DESTINATION:
				bot->trvtmp->destination = text;
				bot->mode = ADD_DATE;
				tg_send_message("Inviami la data del viaggio nel seguente formato GG-MM-AAAA", bot->chat_id);
				break;

			case ADD_DATE: {
				time_t epoch;
				struct tm tm;

				memset(&tm, 0, sizeof(struct tm));
				strptime(text, "%d-%m-%Y", &tm);
				epoch = mktime(&tm);

				if (epoch < time(NULL)) {
					tg_send_message("Non puoi settare una data nel passato, inviami una data corretta", bot->chat_id);
					break;
				}

				char buf[31];
				strftime(buf, sizeof(buf), "%d-%m-%Y", &tm);
				size_t buflen = strlen(buf) + 1;

				bot->trvtmp->date = malloc(buflen);
				memcpy(bot->trvtmp->date, buf, buflen);
				bot->mode = ADD_DRIVER_ID;
				send_drivers(bot->chat_id);
				tg_send_message("Inviami l'ID del guidatore collegato al viaggio", bot->chat_id);
				break;
			}

			case ADD_DRIVER_ID: {
				int id = strtol(text, NULL, 10);
				struct driver *tmp = get_driver(drivers, id);
				if (!tmp) {
					tg_send_message("ID non valido%0AInviami un ID valido", bot->chat_id);
					break;
				}

				int nlen = strlen(tmp->name) + 1;
				bot->trvtmp->driver_name = malloc(nlen);
				strncpy(bot->trvtmp->driver_name, tmp->name, nlen);
				bot->mode = bot->next_mode;

				char msg[511];
				snprintf(msg, 511, "Destinazione: %s%%0AData: %s%%0AGuidatore: %s", bot->trvtmp->destination, bot->trvtmp->date, bot->trvtmp->driver_name);
				tg_send_message(msg, bot->chat_id);
				tg_send_message("Confermi? [S/N]", bot->chat_id);
				break;
			}

			case CONFIRM_ADD_TRV: {
				char response = tolower(text[0]);

				if (response == 's') {
					travels = add_travel(travels, bot->trvtmp);
					bot->mode = DEFAULT;
					send_travels(bot->chat_id);
				}

				else if (response == 'n') {
					bot->mode = DEFAULT;
					free(bot->trvtmp);
					tg_send_message("Inserimento annullato", bot->chat_id);
				}

				else
					tg_send_message("Risposta non valida, scrivi 's' per confermare o 'n' per annullare", bot->chat_id);

				break;
			}

			case CONFIRM_UPD_TRV: {
				char response = tolower(text[0]);

				if (response == 's') {
					update_travels_file(travels);
					bot->mode = DEFAULT;
					send_travels(bot->chat_id);
				}

				else if (response == 'n') {
					bot->mode = DEFAULT;
					tg_send_message("Modifica annullata", bot->chat_id);
				}

				else
					tg_send_message("Risposta non valida, scrivi 's' per confermare o 'n' per annullare", bot->chat_id);

				break;
			}
		}
	}
}


int main(void) {
	FILE *fp = fopen("TOKEN", "rb");

	if (fp == NULL) {
		fputs("Unable to open file TOKEN", stderr);
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	rewind(fp);

	char token[fsize+1];
	memset(token, 0, fsize+1);
	fread(token, 1, fsize+1, fp);
	fclose(fp);

	drivers = load_drivers();
	travels = load_travels();
	run_dispatcher(token);

	return 0;
}
