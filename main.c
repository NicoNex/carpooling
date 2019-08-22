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
	NO_OP,
	
	SELECT_DRIVER,
	RATE,
	
	GET_NAME,
	GET_AGE,
	GET_VEHICLE,
	GET_SEATS,
	
	DEL_DRIVER,
	
	GET_DESTINATION,
	GET_DATE,
	GET_DRIVER_ID,
	GET_PRICE,
	
	MOD_TRAVEL,
	DEL_TRAVEL,

	GET_SORT_MODE,
	GET_QRY,

	CONFIRM
};


enum commands {
	DEFAULT,
	
	RATE_DRV,
	ADD_DRV,
	MOD_DRV,
	DEL_DRV,
	
	ADD_TRV,
	MOD_TRV,
	DEL_TRV,

	SEARCH
};


enum search_travels_modes {
	BY_PRICE,
	BY_RATING
};


struct bot {
	int64_t chat_id;
	int state;
	int mode;
	int sort_mode;
	struct driver *drvtmp;
	struct travel *trvtmp;
};


// volatile for better thread syncronization
volatile list_t drivers;
volatile list_t travels;


struct bot *new_bot(int64_t chat_id) {
	struct bot *bot = malloc(sizeof(struct bot));

	bot->chat_id = chat_id;
	bot->mode = NO_OP;
	bot->state = DEFAULT;
	bot->drvtmp = NULL;

	return bot;
}


static void sort_travels_by_price(list_t node) {
	if (node == NULL)
		return;

	for (list_t tmp = node; tmp && NEXT(tmp); tmp = NEXT(tmp)) {
		struct travel *trv = GET_OBJ(tmp);
		struct travel *ntrv = GET_OBJ(NEXT(tmp));
		
		if (trv->price > ntrv->price) {
			tmp->ptr = ntrv;
			tmp->next->ptr = trv;
		}
	}

	sort_travels_by_price(NEXT(node));
}


static void sort_travels_by_driver_rating(list_t node) {
	if (node == NULL)
		return;

	for (list_t tmp = node; NEXT(tmp); tmp = NEXT(tmp)) {
		struct travel *trv = GET_OBJ(tmp);
		struct travel *ntrv = GET_OBJ(NEXT(tmp));

		struct driver *drv = get_driver_by_token(drivers, trv->token);
		struct driver *ndrv = get_driver_by_token(drivers, ntrv->token);

		if (drv->rating < ndrv->rating) {
			tmp->ptr = ntrv;
			tmp->next->ptr = trv;
		}
	}

	sort_travels_by_driver_rating(NEXT(node));
}


static void send_drivers(const list_t lst, const int64_t chat_id) {
	if (lst == NULL)
		return;

	struct driver *drv = GET_OBJ(lst);
	char msg[511];
	snprintf(msg, 511, 
			"*ID*: %d%%0A*Nome*: %s%%0A*Età*: %d%%0A*Veicolo*: %s%%0A*Posti*: %d%%0A*Valutazione*: %d/10", 
			drv->id, drv->name, drv->age, drv->vehicle, drv->seats, drv->rating);
	
	tg_send_message(msg, chat_id);
	send_drivers(NEXT(lst), chat_id);
}


static void send_travels(const list_t lst, const int64_t chat_id) {
	if (lst == NULL)
		return;

	char msg[511];
	struct travel *trv = GET_OBJ(lst);
	struct driver *drv = get_driver_by_token(drivers, trv->token);
	snprintf(msg, 511, 
			"*ID*: %d%%0A*Destinazione*: %s%%0A*Data*: %s%%0A*Guidatore*: %s%%0A*Valutazione*: %d/10%%0A*Prezzo*: %.2f €", 
			trv->id, trv->destination, trv->date, drv->name, drv->rating, trv->price);
	
	tg_send_message(msg, chat_id);
	send_travels(NEXT(lst), chat_id);
}


static void search_travels(const char *text, int64_t chat_id, int filter) {
	list_t lst = NULL;

	for (list_t tmp = travels; tmp; tmp = NEXT(tmp)) {
		struct travel *trv = GET_OBJ(tmp);

		if (strstr(trv->destination, text))
			lst = list_add(lst, trv);
	}

	switch (filter) {
		case BY_PRICE:
			sort_travels_by_price(lst);
			break;

		case BY_RATING:
			sort_travels_by_driver_rating(lst);
			break;
	}

	send_travels(lst, chat_id);
	dispose_list(lst);
}


void update_bot(struct bot *bot, struct json_object *update) {
	struct json_object *messageobj, *fromobj, *usrobj, *textobj;

	if (json_object_object_get_ex(update, "message", &messageobj)
			&& json_object_object_get_ex(messageobj, "from", &fromobj)
			&& json_object_object_get_ex(messageobj, "text", &textobj)
			&& json_object_object_get_ex(fromobj, "username", &usrobj)) {


		const char *text = json_object_get_string(textobj);
		const char *username = json_object_get_string(usrobj);

		// we need to check for "/annulla" in any case
		if (!strcmp(text, "/annulla")) {
			switch (bot->state) {
				case ADD_DRV:
					free(bot->drvtmp);
					break;

				case ADD_TRV:
					free(bot->trvtmp);
					break;
			}

			bot->state = DEFAULT;
			bot->mode = NO_OP;
			tg_send_message("Azione annullata", bot->chat_id);
			return;
		}

		switch (bot->mode) {
			case NO_OP:
				if (!strcmp(text, "/ping"))
					tg_send_message("Hey!", bot->chat_id);

				else if (!strcmp(text, "/guidatori"))
					send_drivers(drivers, bot->chat_id);

				else if (!strcmp(text, "/valuta")) {
					bot->state = RATE_DRV;
					bot->mode = SELECT_DRIVER;
					tg_send_message("Scrivimi l'ID del guidatore che vuoi valutare", bot->chat_id);
				}

				else if (!strcmp(text, "/viaggi"))
					send_travels(travels, bot->chat_id);

				else if (!strcmp(text, "/cerca")) {
					bot->state = SEARCH;
					bot->mode = GET_SORT_MODE;
					tg_send_message("Vuoi ordinare la lista per prezzo o valutazione del guidatore? [P/V]", bot->chat_id);
				}

				else if (!strcmp(text, "/agg_guidatore")) {
					bot->state = ADD_DRV;
					bot->mode = GET_NAME;
					bot->drvtmp = calloc(1, sizeof(struct driver));
					bot->drvtmp->token = time(NULL);
					tg_send_message("Scrivimi il nome del guidatore che vuoi aggiungere", bot->chat_id);
				}

				else if (!strcmp(text, "/mod_guidatore")) {
					bot->state = MOD_DRV;
					bot->mode = SELECT_DRIVER;
					tg_send_message("Scrivimi l'ID del guidatore che vuoi modificare", bot->chat_id);
				}

				else if (!strcmp(text, "/canc_guidatore")) {
					bot->state = DEL_DRV;
					bot->mode = DEL_DRIVER;
					send_drivers(drivers, bot->chat_id);
					tg_send_message("Scrivimi l'ID del guidatore che vuoi cancellare", bot->chat_id);
				}

				else if (!strcmp(text, "/agg_viaggio")) {
					bot->state = ADD_TRV;
					bot->mode = GET_DESTINATION;
					bot->trvtmp = calloc(1, sizeof(struct travel));
					tg_send_message("Scrivimi la destinazione del nuovo viaggio", bot->chat_id);
				}

				else if (!strcmp(text, "/mod_viaggio")) {
					bot->state = MOD_TRV;
					bot->mode = MOD_TRAVEL;
					tg_send_message("Scrivimi l'ID del viaggio che vuoi modificare", bot->chat_id);
				}

				else if (!strcmp(text, "/canc_viaggio")) {
					bot->state = DEL_TRV;
					bot->mode = DEL_TRAVEL;
					send_travels(travels, bot->chat_id);
					tg_send_message("Scrivimi l'ID del viaggio che vuoi cancellare", bot->chat_id);
				}

				else if (!strcmp(text, "/miglior_guidatore")) {
					struct driver *best = GET_OBJ(drivers);
					
					for (list_t drv = drivers; drv; drv = NEXT(drv)) {
						struct driver *tmp = GET_OBJ(drv);

						if (tmp->rating > best->rating)
							best = tmp;
					}

					char msg[512];
					snprintf(msg, 512, 
							"*ID*: %d%%0A*Nome*: %s%%0A*Età*: %d%%0A*Veicolo*: %s%%0A*Posti*: %d%%0A*Valutazione*: %d%%0A%%0A", 
							best->id, best->name, best->age, best->vehicle, best->seats, best->rating);
					
					tg_send_message(msg, bot->chat_id);
				}

				break;

			case SELECT_DRIVER: {
				int id = strtol(text, NULL, 10);
				bot->drvtmp = get_driver_by_id(drivers, id);

				if (bot->drvtmp == NULL) {
					tg_send_message("ID incorretto.%0AScrivi solo il numero dell'ID del guidatore da valutare", bot->chat_id);
					break;
				}

				char msg[511] = {'\0'};

				switch (bot->state) {
				case RATE_DRV:
					snprintf(msg, 511, "Scrivi la valutazione da dare a %s, da 1 a 10", bot->drvtmp->name);
					bot->mode = RATE;
					break;

				case MOD_DRV:
					snprintf(msg, 511, "Scrivi il nuovo nome di %s", bot->drvtmp->name);
					bot->mode = GET_NAME;
					break;
				}

				tg_send_message(msg, bot->chat_id);
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
				bot->mode = NO_OP;
				bot->state = DEFAULT;
				tg_send_message("Grazie per il tuo feedback!", bot->chat_id);
				break;
			}

			case GET_NAME: {
				size_t len = strlen(text) + 1;
				bot->drvtmp->name = malloc(len);
				strncpy(bot->drvtmp->name, text, len);
				bot->mode = GET_AGE;
				tg_send_message("Inviami l'età del guidatore scrivendo solo il numero degli anni", bot->chat_id);
				break;
			}

			case GET_AGE: {
				char msg[512];
				int age = strtol(text, NULL, 10);

				if (!age) {
					snprintf(msg, 512, "Età incorretta.%%0AScrivi l'età di %s mandando solo il numero degli anni", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					break;
				}

				bot->drvtmp->age = age;
				bot->mode = GET_VEHICLE;
				snprintf(msg, 512, "Inviami il tipo di veicolo guidato da %s", bot->drvtmp->name);
				tg_send_message(msg, bot->chat_id);
				break;
			}

			case GET_VEHICLE: {
				size_t len = strlen(text) + 1;
				bot->drvtmp->vehicle = malloc(len);
				strncpy(bot->drvtmp->vehicle, text, len);
				bot->mode = GET_SEATS;
				tg_send_message("Inviami il numero di posti dispobibili", bot->chat_id);
				break;
			}

			case GET_SEATS: {
				char msg[511];
				int seats = strtol(text, NULL, 10);

				if (seats < 1) {
					snprintf(msg, 511, "Numero posti non valido, immetti il numero di posti disponibili per %s", bot->drvtmp->name);
					tg_send_message(msg, bot->chat_id);
					break;
				}

				bot->drvtmp->seats = seats;

				snprintf(msg, 511, "*Nome*: %s%%0A*Età*: %d%%0A*Veicolo*: %s%%0A*Posti*: %d", bot->drvtmp->name, bot->drvtmp->age, bot->drvtmp->vehicle, bot->drvtmp->seats);
				tg_send_message(msg, bot->chat_id);
				tg_send_message("Confermi? [S/N]", bot->chat_id);
				bot->mode = CONFIRM;
				break;
			}

			case DEL_DRIVER: {
				int id = strtol(text, NULL, 10);
				struct driver *drv = get_driver_by_id(drivers, id);
				
				if (!drv) {
					tg_send_message("ID non valido%0AInviami un ID valido", bot->chat_id);
					break;
				}

				travels = del_travels_with_token(travels, drv->token);
				drivers = del_driver(drivers, id);
				tg_send_message("Guidatore cancellato", bot->chat_id);
				bot->mode = NO_OP;
				bot->state = DEFAULT;
				break;
			}

			case DEL_TRAVEL: {
				int id = strtol(text, NULL, 10);
				if (!get_travel(travels, id)) {
					tg_send_message("ID non valido%0AInviami un ID valido", bot->chat_id);
					break;
				}

				travels = del_travel(travels, id);
				tg_send_message("Viaggio cancellato", bot->chat_id);
				bot->mode = NO_OP;
				bot->state = DEFAULT;
				break;
			}

			case MOD_TRAVEL: {
				int id = strtol(text, NULL, 10);
				bot->trvtmp = get_travel(travels, id);

				if (bot->trvtmp == NULL) {
					tg_send_message("ID incorretto.%0AScrivi solo il numero dell'ID del viaggio da modificare", bot->chat_id);
					break;
				}

				bot->mode = GET_DESTINATION;
				tg_send_message("Scrivimi la destinazione del nuovo viaggio", bot->chat_id);
				break;
			}

			case GET_DESTINATION: {
				size_t len = strlen(text) + 1;
				bot->trvtmp->destination = malloc(len);
				strncpy(bot->trvtmp->destination, text, len);
				bot->mode = GET_DATE;
				tg_send_message("Inviami la data del viaggio nel formato GG-MM-AAAA", bot->chat_id);
				break;
			}

			case GET_DATE: {
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
				bot->mode = GET_DRIVER_ID;
				send_drivers(drivers, bot->chat_id);
				tg_send_message("Inviami l'ID del guidatore collegato al viaggio", bot->chat_id);
				break;
			}

			case GET_DRIVER_ID: {
				int id = strtol(text, NULL, 10);
				struct driver *tmp = get_driver_by_id(drivers, id);
				if (!tmp) {
					tg_send_message("ID non valido%0AInviami un ID valido", bot->chat_id);
					break;
				}

				bot->trvtmp->token = tmp->token;
				printf("%ld\n", bot->trvtmp->token);
				tg_send_message("Inviami il prezzo del viaggio", bot->chat_id);
				bot->mode = GET_PRICE;
				break;
			}

			case GET_PRICE: {
				float price = atof(text);
				bot->trvtmp->price = price;
				bot->mode = CONFIRM;

				struct driver *drv = get_driver_by_token(drivers, bot->trvtmp->token);
				char msg[511];

				snprintf(msg, 511, 
					"*Destinazione*: %s%%0A*Data*: %s%%0A*Guidatore*: %s%%0A*Prezzo*: %.2f €", 
					bot->trvtmp->destination, bot->trvtmp->date, drv->name, bot->trvtmp->price);
				
				tg_send_message(msg, bot->chat_id);
				tg_send_message("Confermi? [S/N]", bot->chat_id);
				break;
			}

			case GET_SORT_MODE: {
				char response = tolower(text[0]);

				switch (response) {
					case 'p':
						bot->sort_mode = BY_PRICE;
						break;

					case 'v':
						bot->sort_mode = BY_RATING;
						break;

					default:
						tg_send_message(
							"Risposta non valida, scrivi 'P' per ordinare i viaggi per prezzo,"
							"'V' per ordinarli in base alla valutazione del guidatore", bot->chat_id);
						return;
				}

				bot->mode = GET_QRY;
				tg_send_message("Scrivimi il nome della località da cercare", bot->chat_id);
				break;
			}

			case GET_QRY:
				search_travels(text, bot->chat_id, bot->sort_mode);
				bot->mode = NO_OP;
				bot->state = DEFAULT;
				break;

			case CONFIRM: {
				char response = tolower(text[0]);

				if (response == 's') {					
					switch (bot->state) {
						case ADD_DRV:
							drivers = add_driver(drivers, bot->drvtmp);
							send_drivers(drivers, bot->chat_id);
							break;

						case MOD_DRV:
							update_drivers_file(drivers);
							send_drivers(drivers, bot->chat_id);
							break;

						case ADD_TRV:
							travels = add_travel(travels, bot->trvtmp);
							send_travels(travels, bot->chat_id);
							break;

						case MOD_TRV:
							update_travels_file(travels);
							send_travels(travels, bot->chat_id);
							break;
					}
					bot->mode = NO_OP;
					bot->state = DEFAULT;
				}

				else if (response == 'n') {
					switch (bot->state) {
						case ADD_TRV:
							free(bot->trvtmp);
							break;

						case ADD_DRV:
							free(bot->drvtmp);
							break;
					}

					bot->mode = NO_OP;
					bot->state = DEFAULT;
					tg_send_message("Azione annullata", bot->chat_id);
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
