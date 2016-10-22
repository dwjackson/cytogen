#include <stdbool.h>
#include <stdlib.h>

enum cyjson_data_type {
    CYJSON_OBJECT,
    CYJSON_OBJECT_BEGIN,
    CYJSON_OBJECT_END,

    CYJSON_ARRAY,
    CYJSON_ARRAY_BEGIN,
    CYJSON_ARRAY_END,

    CYJSON_STRING_DELIMITER,
    CYJSON_STRING,

    CYJSON_NUMBER,
    CYJSON_BOOLEAN,
    CYJSON_NULL,

    CYJSON_NONE
};

union cyjson_value {
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
    enum cyjson_data_type parent_data_type;
    enum cyjson_data_type token_data_type;
    union cyjson_value value;
};
typedef struct cyjson_parser cyjson_parser_t;

int
cyjson_parser_init(cyjson_parser_t *parser, const char *json);

int
cyjson_parse(cyjson_parser_t *parser);
