/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2021 David Jackson
 */

#include "cyjson.h"
#include "cyto_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctache/ctache.h>

static char
*read_file(const char *file_name)
{
    int ch;
    char *buf;
    size_t buf_size;
    size_t length;
    FILE *fp;

    fp = fopen(file_name, "r");
    if (fp == NULL) {
        return NULL;
    }

    buf_size = 10;
    length = 0;
    buf = malloc(buf_size);
    while ((ch = fgetc(fp)) != EOF && !feof(fp)) {
        if (length + 1 >= buf_size - 1) { /* -1 is for '\0' */
            buf_size *= 2;
            buf = realloc(buf, buf_size);
        }
        buf[length] = ch;
        length++;
    }
    buf[length] = '\0';

    fclose(fp);

    return buf;
}

static bool
config_is_valid(struct cyto_config *config)
{
    return config->title != NULL
        && config->url != NULL
        && config->author != NULL;
}

static void
read_config_item(char **dest, const char *value)
{
	*dest = malloc(strlen(value) + 1);
	strcpy(*dest, value);
}

int
cyto_config_read(const char *file_name, struct cyto_config *config)
{
    char *json;
    cyjson_parser_t parser;
    enum cyjson_event_type event_type;
    int parse_ok;
    char *key;
    char *value;
    int status;

    config->title = NULL;
    config->url = NULL;
    config->author = NULL;

    json = read_file(file_name);
    if (json == NULL) {
        return -1;
    }
    cyjson_parser_init(&parser, json);

    key = NULL;
    parse_ok = cyjson_parse(&parser);
    event_type = cyjson_get_event_type(parser);
    while (parse_ok != -1 && event_type != CYJSON_EVENT_OBJECT_END) {
        if (event_type == CYJSON_EVENT_OBJECT_KEY) {
            if (key != NULL) {
                free(key);
            }
            key = strdup(cyjson_get_string(parser));
        } else if (event_type == CYJSON_EVENT_OBJECT_VALUE) {
            value = cyjson_get_string(parser);
            if (strcmp(key, "title") == 0) {
                read_config_item(&(config->title), value);
            } else if (strcmp(key, "url") == 0) {
                read_config_item(&(config->url), value);
            } else if (strcmp(key, "author") == 0) {
                read_config_item(&(config->author), value);
            }
        }
        parse_ok = cyjson_parse(&parser);
        event_type = cyjson_get_event_type(parser);
    }
    if (key != NULL) {
        free(key);
    }

    if (config_is_valid(config)) {
        status = 0;
    } else {
        status = -1;
    }

    cyjson_parser_destroy(&parser);
    free(json);

    return status;
}

void
cyto_config_destroy(struct cyto_config *config)
{
    if (config->title != NULL) {
        free(config->title);
    }
    if (config->url != NULL) {
        free(config->url);
    }
    if (config->author != NULL) {
        free(config->author);
    }
}

ctache_data_t
*cyto_config_to_ctache_data(struct cyto_config *config)
{
	ctache_data_t *config_data = ctache_data_create_hash();
	if (config_data == NULL) {
		return NULL;
	}

	char *title = config->title;
	ctache_data_t *title_data = ctache_data_create_string(title, strlen(title));
	if (title == NULL) {
		ctache_data_destroy(config_data);
		return NULL;
	}
	ctache_data_hash_table_set(config_data, "title", title_data);

	char *author = config->author;
	ctache_data_t *author_data = ctache_data_create_string(author, strlen(author));
	if (author_data == NULL) {
		ctache_data_destroy(config_data);
		return NULL;
	}
	ctache_data_hash_table_set(config_data, "author", author_data);

	char *url = config->url;
	ctache_data_t *url_data = ctache_data_create_string(url, strlen(url));
	if (url_data == NULL) {
		ctache_data_destroy(config_data);
		return NULL;
	}
	ctache_data_hash_table_set(config_data, "url", url_data);

	return config_data;
}
