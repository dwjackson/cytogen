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
    parser->value.null = NULL;

    return 0;
}

static void
free_string_if_necessary(cyjson_parser_t *parser)
{
    if (parser->data_type == CYJSON_STRING) {
        free(parser->value.string.buffer);
        parser->value.string.length = 0;
        parser->value.string.bufsize = 0;
    }
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

        union cyjson_value *value = &(parser->value);
        value->string.bufsize = DEFAULT_STRING_BUFLEN;
        value->string.length = 0;
        value->string.buffer = malloc(parser->value.string.bufsize);

        (parser->json)++; /* Skip past the first quotation mark (") */
        int index = 0;
        while ((ch = *(parser->json++)) != '"') {
            if (value->string.length + 1 < value->string.bufsize) {
                size_t bufsize = value->string.bufsize * 2;
                value->string.bufsize = bufsize;
                value->string.buffer = realloc(value->string.buffer, bufsize);
            }
            (value->string.buffer)[index] = ch;
            index++;
        }
        (value->string.buffer)[index] = '\0';
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
    return parser.value.string.buffer;
}

enum cyjson_event_type
cyjson_get_event_type(cyjson_parser_t parser)
{
    return parser.event_type;
}
