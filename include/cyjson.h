/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef CYJSON_H
#define CYJSON_H

#include <stdbool.h>
#include <stdlib.h>

enum cyjson_data_type {
    CYJSON_OBJECT,
    CYJSON_ARRAY,
    CYJSON_STRING,
    CYJSON_NUMBER,
    CYJSON_BOOLEAN,
    CYJSON_NULL,
    CYJSON_NONE /* Not an actual data type, used for signaling */
};

enum cyjson_event_type {
    CYJSON_EVENT_OBJECT_BEGIN,
    CYJSON_EVENT_OBJECT_END,
    CYJSON_EVENT_OBJECT_KEY,
    CYJSON_EVENT_OBJECT_VALUE,
    CYJSON_EVENT_ARRAY_BEGIN,
    CYJSON_EVENT_ARRAY_END,
    CYJSON_EVENT_DATA,
    CYJSON_EVENT_NONE
};

union cyjson_data {
    double number;
    bool boolean;
    void *null;
    union cyjson_string {
        char *buffer;
        size_t length;
        size_t bufsize;
    } string;
};

struct cyjson_parser {
    const char *json;
    enum cyjson_event_type event_type;
    enum cyjson_data_type data_type;
    union cyjson_data data;
};
typedef struct cyjson_parser cyjson_parser_t;

int
cyjson_parser_init(cyjson_parser_t *parser, const char *json);

void
cyjson_parser_destroy(cyjson_parser_t *parser);

int
cyjson_parse(cyjson_parser_t *parser);

enum cyjson_data_type
cyjson_get_data_type(cyjson_parser_t parser);

char
*cyjson_get_string(cyjson_parser_t parser);

enum cyjson_event_type
cyjson_get_event_type(cyjson_parser_t parser);

#endif /* CYJSON_H */
