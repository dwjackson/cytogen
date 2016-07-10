/*
 * Markdown Grammar
 *
 * document = block, more blocks
 * more blocks = block end, block, more blocks | ""
 * block = paragraph | header
 * paragraph = text and inline, more text and inline
 * text and inline = string
 * text and inline = inline
 * inline = "`", string, "`"
 * more text and inline = "\n", text and inline, more text and inline
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct cymkd_parser {
    const char *str_pos;
};

static bool
match(struct cymkd_parser *parser, char ch)
{
    if (parser->str_pos == ch) {
        (parser->str_pos)++;
        return true;
    } else {
        fprintf(stderr, "ERROR\n");
        return false;
    }
}

static bool
block(struct cymkd_parser *parser)
{
    bool success;
    // TODO
}

static bool 
document(struct cymkd_parser *parser)
{
    bool success;
    success = block(parser);
    if (success) {
        success = more_blocks(parser);
    }
    return success;
}

void
cymkd_parse(const char *str, size_t str_len, FILE *out_fp)
{
    struct cymkd_parser parser;
    parser.str_pos = str;

    // TODO
}
