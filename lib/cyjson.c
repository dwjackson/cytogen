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
