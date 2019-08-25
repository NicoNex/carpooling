/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */


#include <json.h>
#include <stdio.h>
#include "include/filehandler.h"


json_object *load_json_from_file(const char *filepath) {
	FILE *fp;
	char *buffer;
	long file_size;
	struct json_object *json;

	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "error: cannot open file %s", filepath);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	if (file_size > 0) {
		buffer = malloc(file_size);
		fread(buffer, file_size, 1, fp);
		json = json_tokener_parse(buffer);
		free(buffer);
	}
	fclose(fp);

	return json;
}

