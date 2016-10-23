/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cyjson.h"

#define DEFAULT_STRING_BUFLEN 20

int
cyjson_parser_init(cyjson_parser_t *parser, const char *json)
{
    parser->json = json;
    parser->event_type = CYJSON_EVENT_NONE;
    parser->data_type = CYJSON_NONE;
    parser->data.null = NULL;

    return 0;
}

static void
free_string_if_necessary(cyjson_parser_t *parser)
{
    if (parser->data_type == CYJSON_STRING) {
        free(parser->data.string.buffer);
        parser->data.string.length = 0;
        parser->data.string.bufsize = 0;
    }
}

void
cyjson_parser_destroy(cyjson_parser_t *parser)
{
    free_string_if_necessary(parser);
}

int
cyjson_parse(cyjson_parser_t *parser)
{
    const char *json = parser->json;
    int ch = *json;
    if (ch == '{') {
        free_string_if_necessary(parser);
        parser->event_type = CYJSON_EVENT_OBJECT_BEGIN;
        parser->data_type = CYJSON_OBJECT;
    } else if (ch == '}') {
        free_string_if_necessary(parser);
        parser->event_type = CYJSON_EVENT_OBJECT_END;
        parser->data_type = CYJSON_NONE;
    } else if (ch == '[') {
        free_string_if_necessary(parser);
        parser->event_type = CYJSON_EVENT_ARRAY_BEGIN;
        parser->data_type = CYJSON_ARRAY;
    } else if (ch == ']') {
        free_string_if_necessary(parser);
        parser->event_type = CYJSON_EVENT_ARRAY_END;
        parser->data_type = CYJSON_NONE;
    } else if (ch == '"') {
        free_string_if_necessary(parser);

        enum cyjson_event_type prev_event_type = parser->event_type;
        if (prev_event_type == CYJSON_EVENT_OBJECT_BEGIN) {
            parser->event_type = CYJSON_EVENT_OBJECT_KEY;
        } else if (prev_event_type == CYJSON_EVENT_OBJECT_KEY) {
            parser->event_type = CYJSON_EVENT_OBJECT_VALUE;
        } else if (prev_event_type == CYJSON_EVENT_OBJECT_VALUE) {
            parser->event_type = CYJSON_EVENT_OBJECT_KEY;
        } else {
            parser->event_type = CYJSON_EVENT_DATA;
        }
        parser->data_type = CYJSON_STRING;

        parser->data.string.bufsize = DEFAULT_STRING_BUFLEN;
        parser->data.string.length = 0;
        parser->data.string.buffer = malloc(parser->data.string.bufsize);
        if (parser->data.string.buffer == NULL) {
            abort();
        }

        (parser->json)++; /* Skip past the first quotation mark (") */
        while ((ch = *(parser->json++)) != '"') {
            if (parser->data.string.length + 1 >= parser->data.string.bufsize) {
                size_t bufsize = parser->data.string.bufsize * 2;
                parser->data.string.bufsize = bufsize;
                parser->data.string.buffer = realloc(parser->data.string.buffer, bufsize);
            }
            (parser->data.string.buffer)[parser->data.string.length] = ch;
            parser->data.string.length++;
        }
        (parser->data.string.buffer)[parser->data.string.length] = '\0';
    }

    (parser->json)++;
    ch = *(parser->json);
    while (ch == ' ' || ch == '\t') { /* Ignore whitespace */
        (parser->json)++;
        ch = *(parser->json);
    }

    return 0;
}

enum cyjson_data_type
cyjson_get_data_type(cyjson_parser_t parser)
{
    return parser.data_type;
}

char
*cyjson_get_string(cyjson_parser_t parser)
{
    return parser.data.string.buffer;
}

enum cyjson_event_type
cyjson_get_event_type(cyjson_parser_t parser)
{
    return parser.event_type;
}
