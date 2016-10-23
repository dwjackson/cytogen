/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cyjson.h"
#include "cyto_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        if (length + 1 >= buf_size) {
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

int
cyto_config_read(const char *file_name, struct cyto_config *config)
{
    char *json;
    cyjson_parser_t parser;
    enum cyjson_event_type event_type;
    int parse_ok;
    char *key;
    char *value;

    json = read_file(file_name);
    if (json == NULL) {
        return -1;
    }
    cyjson_parser_init(&parser, json);

    key = NULL;
    parse_ok = cyjson_parse(&parser);
    event_type = cyjson_get_event_type(parser);
    while (parse_ok && event_type != CYJSON_EVENT_OBJECT_END) {
        if (event_type == CYJSON_EVENT_OBJECT_KEY) {
            key = cyjson_get_string(parser);
        } else if (event_type == CYJSON_EVENT_OBJECT_VALUE) {
            value = cyjson_get_string(parser);
            if (strcmp(key, "title") == 0) {
                config->title = malloc(strlen(value) + 1);
                strcpy(config->title, value);
            } else if (strcmp(key, "url") == 0) {
                config->url = malloc(strlen(value) + 1);
                strcpy(config->title, value);
            } else if (strcmp(key, "author") == 0) {
                config->author = malloc(strlen(value) + 1);
                strcpy(config->title, value);
            }
        }
        parse_ok = cyjson_parse(&parser);
        event_type = cyjson_get_event_type(parser);
    }

    cyjson_parser_destroy(&parser);

    return 0;
}

void
cyto_config_destroy(struct cyto_config *config)
{
    free(config->title);
    free(config->url);
    free(config->author);
}
