/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

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
 * header = header prefix, " ", string
 * header prefix = { "#" }, string | string, "\n", underline
 * underline = { "-" } | { "=" }
 * inline = italics | bold | inline code
 * italics = ("*" | "_"), string, ("*" | "_")
 * bold = ("**" | "__"), string, ("**" | "__")
 * inline code = "`", string, "`"
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct cymkd_parser {
    const char *str_pos;
    FILE *out_fp;
};

static void
parser_emit_string(struct cymkd_parser *parser, const char *str)
{
    fprintf(parser->out_fp, "%s", str);
}

static void
parser_emit_string(struct cymkd_parser *parser, int ch)
{
    fprintf(parser->out_fp, "%c", ch);
}

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
paragraph(struct cymkd_parser *parser)
{
    // TODO
}

static bool
header_prefix(struct cymkd_parser *parser)
{
    int header_level = 0;
    while (match(parser, '#')) {
        header_level++;
    }
    if (header_level == 0) {
        return false;
    }
    // TODO
}

static bool
header(struct cymkd_parser *parser)
{
    if (header_prefix(parser)) {
        // TODO
    }
    return false;
}

static bool
block(struct cymkd_parser *parser)
{
    bool success;
    success = paragraph(parser);
    if (!success) {
        success = header(parser);
    }
    return success;
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
    parser.out_fp = out_fp;
    if (!document(parser)) {
        fprintf(stderr, "ERROR\n");
    }
}
