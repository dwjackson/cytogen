/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cyjson.h"

int
cyjson_parser_init(cyjson_parser_t *parser, const char *json)
{
    parser->json = json;
    parser->parent_data_type = CYJSON_NONE;
    parser->token_data_type = CYJSON_NONE;
    parser->value.null = NULL;

    return 0;
}

int
cyjson_parse(cyjson_parser_t *parser)
{
    const char *json = parser->json;
    int ch = *json;
    if (ch == '{' && parser->parent_data_type != CYJSON_STRING) {
        parser->token_data_type = CYJSON_OBJECT_BEGIN;
        parser->parent_data_type = CYJSON_OBJECT;
    } else if (ch == '}' && parser->parent_data_type != CYJSON_STRING) {
        parser->token_data_type = CYJSON_OBJECT_END;
        parser->parent_data_type = CYJSON_NONE;
    } else if (ch == '[' && parser->parent_data_type != CYJSON_STRING) {
        parser->token_data_type = CYJSON_ARRAY_BEGIN;
        parser->parent_data_type = CYJSON_ARRAY;
    } else if (ch == ']' && parser->parent_data_type != CYJSON_STRING) {
        parser->token_data_type = CYJSON_ARRAY_END;
        parser->parent_data_type = CYJSON_NONE;
    }
    // TODO

    (parser->json)++;

    return 0;
}

enum cyjson_data_type
cyjson_token_data_type(cyjson_parser_t parser)
{
    return parser.token_data_type;
}
