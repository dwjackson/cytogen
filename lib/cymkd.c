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
#include <stdarg.h>

#define BUFSIZE 1024

struct cymkd_parser {
    const char *str_pos;
    size_t str_len;
    int str_index;
    FILE *out_fp;
};

static void
parser_emit_string(struct cymkd_parser *parser, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(parser->out_fp, fmt, ap);
}

static void
parser_emit_char(struct cymkd_parser *parser, int ch)
{
    fprintf(parser->out_fp, "%c", ch);
}

static bool
match(struct cymkd_parser *parser, char ch)
{
    if (parser->str_index >= parser->str_len) {
        return -1;
    }
    if (*(parser->str_pos) == ch) {
        (parser->str_pos)++;
        (parser->str_index)++;
        return true;
    } else {
        return false;
    }
}

static int
consume(struct cymkd_parser *parser)
{
    int ch;
    if (parser->str_index >= parser->str_len) {
        return -1;
    }
    ch = *(parser->str_pos);
    (parser->str_pos)++;
    (parser->str_index)++;
    return ch;
}

static int
next(struct cymkd_parser *parser)
{
    int ch;
    if (parser->str_index >= parser->str_len) {
        return -1;
    }
    ch = *(parser->str_pos);
    return ch;
}

static bool
is_inline_start(int ch)
{
    bool is_inline;
    switch(ch) {
    case '*':
    case '_':
    case '`':
        is_inline = true;
        break;
    default:
        is_inline = false;
    }
    return is_inline;
}

static bool
bold(struct cymkd_parser *parser)
{
    bool success;
    int start_ch;
    int ch;
    const char *str_pos = parser->str_pos;

    success = match(parser, '*');
    if (!success) {
        success = match(parser, '_');
        if (!success) {
            parser->str_pos = str_pos; /* Go back */
            return false;
        }
        success = match(parser, '_');
        if (!success) {
            parser->str_pos = str_pos; /* Go back */
            return false;
        }
        start_ch = '_';
    } else {
        success = match(parser, '*');
        if (!success) {
            parser->str_pos = str_pos; /* Go back */
            return false;
        }
        start_ch = '*';
    }

    parser_emit_string(parser, "<strong>");
    while ((ch = next(parser)) != start_ch && ch != -1) {
        parser_emit_char(parser, ch);
        consume(parser);
    }

    if (!match(parser, start_ch)) {
        parser->str_pos = str_pos; /* Go back */
        return false;
    }
    success = match(parser, start_ch);
    if (success) {
        parser_emit_string(parser, "</strong>");
    }
    return success;
}

static bool
italics(struct cymkd_parser *parser)
{
    bool success;
    int start_ch;
    int ch;

    success = match(parser, '*');
    if (success) {
        start_ch = '*';
    } else {
        success = match(parser, '_');
        if (success) {
            start_ch = '_';
        }
    }
    if (!success) {
        return false;
    }

    parser_emit_string(parser, "<em>");
    while ((ch = consume(parser)) != start_ch && ch >= 0) {
        parser_emit_char(parser, ch);
    }
    if (ch == start_ch) {
        parser_emit_string(parser, "</em>");
        return true;
    }
    return false;
}

static bool
inline_code(struct cymkd_parser *parser)
{
    int ch;
    if (!match(parser, '`')) {
        return false;
    }
    parser_emit_string(parser, "<pre><code>");
    while ((ch = consume(parser)) != '`' && ch != -1) {
        parser_emit_char(parser, ch);
    }
    if (ch != '`') {
        return false;
    }
    parser_emit_string(parser, "</pre></code>");
    return true;
}

static bool
inline_section(struct cymkd_parser *parser)
{
    if (!bold(parser)) {
        if (!italics(parser)) {
            if (!inline_code(parser)) {
                return false;
            }
        }
    }
    return true;
}

static bool
paragraph(struct cymkd_parser *parser)
{
    int ch;
    parser_emit_string(parser, "<p>");
    while (true) {
        while ((ch = next(parser)) != '\n' && ch != -1) {
            if (is_inline_start(ch)) {
                if (!inline_section(parser)) {
                    fprintf(stderr, "Invalid inline section\n");
                    return false;
                } else {
                    parser_emit_char(parser, next(parser));
                }
            } else {
                parser_emit_char(parser, ch);
            }
            consume(parser); /* Move to the next input character */
        }
        if (match(parser, '\n') && next(parser) == '\n') {
            consume(parser);
            parser_emit_string(parser, "</p>");
            break;
        } else if (next(parser) == -1) {
            return false;
        } else {
            parser_emit_char(parser, '\n');
        }
    }
    return true;
}

static bool
header_prefix(struct cymkd_parser *parser, int *header_level_ptr)
{
    int header_level = 0;
    while (match(parser, '#')) {
        header_level++;
    }
    if (header_level == 0) {
        return false;
    }
    if (!match(parser, ' ')) {
        fprintf(stderr, "ERROR: Expected space after #\n");
        return false;
    }
    parser_emit_string(parser, "<h%d>", header_level);
    *header_level_ptr = header_level;
    return true;
}

static bool
header(struct cymkd_parser *parser)
{
    int ch;
    int header_level;
    if (header_prefix(parser, &header_level)) {
        while ((ch = consume(parser)) != '\n') {
            parser_emit_char(parser, ch);
        }
        ch = consume(parser);
        if (ch != '\n') {
            printf("ERROR: Header must end with two newlines\n");
            return false;
        }
        parser_emit_string(parser, "</h%d>", header_level);
        return true;
    }
    return false;
}

static bool
block(struct cymkd_parser *parser)
{
    bool success;
    success = header(parser);
    if (!success) {
        success = paragraph(parser);
    }
    return success;
}

static bool
newlines(struct cymkd_parser *parser)
{
    if (next(parser) == '\n') {
        consume(parser);
        return true;
    }
    return false;
}

static bool
more_blocks(struct cymkd_parser *parser)
{
    if (next(parser) != -1) {
        block(parser);
        while (newlines(parser)) {
            parser_emit_char(parser, '\n');
        }
        return more_blocks(parser);
    }
    return true;
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
cymkd_render(const char *str, size_t str_len, FILE *out_fp)
{
    struct cymkd_parser parser;
    parser.str_pos = str;
    parser.str_len = str_len;
    parser.str_index = 0;
    parser.out_fp = out_fp;
    if (!document(&parser)) {
        fprintf(stderr, "ERROR: Markdown document is invalid\n");
    }
}

void
cymkd_render_file(FILE *in_fp, FILE *out_fp)
{
    int ch;
    char *content;
    size_t content_len;
    size_t content_bufsize;

    content_bufsize = BUFSIZE;
    content = malloc(content_bufsize);
    content_len = 0;
    while ((ch = fgetc(in_fp)) != EOF) {
        if (content_len + 1 >= content_bufsize) {
            content_bufsize *= 2;
            content = realloc(content, content_bufsize); 
        }
        content[content_len] = ch;
        content_len++;
    }
    content[content_len] = '\0';
    cymkd_render(content, content_len, out_fp);
    free(content);
}
