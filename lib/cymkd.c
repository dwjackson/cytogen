/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2018 David Jackson
 */

/*
 * Markdown Grammar
 *
 * document = block, more blocks
 * more blocks = block end, block, more blocks | ""
 * block = paragraph | header | unordered list | block quote | code block
 * paragraph = text and inline, more text and inline
 * text and inline = string
 * text and inline = inline
 * more text and inline = "\n", text and inline, more text and inline
 * header = header prefix, " ", string
 * header prefix = { "#" }, string | string, "\n", underline
 * unordered list = unordered list line, more unordered list lines
 * unordered list line = ("*" | "-") text and inline
 * more unordered list lines = "\n", unordered list line, more unordered list lines
 * block quote = block quote line, more block quote lines
 * block quote line = ">" string
 * code block = "```", { string }, "\n", code line, more code lines, "\n", "```"
 * more block quote lines = "\n", block quote line, more block quote lines | ""
 * underline = { "-" } | { "=" }
 * inline = italics | bold | inline code | link | image
 * italics = ("*" | "_"), string, ("*" | "_")
 * bold = ("**" | "__"), string, ("**" | "__")
 * inline code = "`", string, "`"
 * link = "[", string, "]", "(", string, ")"
 * image = "!", "[", string, "]", "(", string, ")"
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>

#define BUFSIZE 1024
#define ESCAPE_CHAR '\\'

struct cymkd_parser {
    const char *str_pos;
    size_t str_len;
    int str_index;
    FILE *out_fp;
    int out_fd;
    int line;
    int col;
    const char *file_name;
};

struct cymkd_parser_position {
    const char *str_pos;
    int str_index;
};

static void
cymkd_error(struct cymkd_parser *parser, const char *err_fmt, ...)
{
    const char *file_name = parser->file_name;
    int line = parser->line;
    int col = parser->col;
    char *err_str;
    va_list ap;
    va_start(ap, err_fmt);
    vasprintf(&err_str, err_fmt, ap);
    va_end(ap);
    char fmt[] = "Error in %s at line %d, col %d: %s";
    fprintf(stderr, fmt, file_name, line, col, err_str);
    free(err_str);
}

static void
parser_emit_string(struct cymkd_parser *parser, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (parser->out_fp != NULL) {
        vfprintf(parser->out_fp, fmt, ap);
    } else {
        char *str;
        vasprintf(&str, fmt, ap);
        write(parser->out_fd, str, strlen(str));
        free(str);
    }
}

static void
parser_emit_char(struct cymkd_parser *parser, int ch)
{
    if (parser->out_fp != NULL) {
        fprintf(parser->out_fp, "%c", ch);
    } else {
        write(parser->out_fd, &ch, 1);
    }
}

static bool
match(struct cymkd_parser *parser, char ch)
{
    if (parser->str_index >= parser->str_len) {
        return false;
    }
    if (*(parser->str_pos) == ch) {
        (parser->str_pos)++;
        (parser->str_index)++;
        if (ch == '\n') {
            parser->line++;
            parser->col = 1;
        } else {
            parser->col++;
        }
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
    if (ch == '\n') {
        parser->line++;
        parser->col = 1;
    } else {
        parser->col++;
    }
    return ch;
}

/* Go backward in the parser by one byte */
static void
unconsume(struct cymkd_parser *parser)
{
    if (parser->str_index <= 0) {
        return;
    }
    (parser->str_pos)--;
    (parser->str_index)--;
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

static struct cymkd_parser_position
save_position(struct cymkd_parser *parser)
{
    struct cymkd_parser_position position;
    position.str_index = parser->str_index;
    position.str_pos = parser->str_pos;
    return position;
}

static void
restore_position(struct cymkd_parser *parser,
                 struct cymkd_parser_position position)
{
    parser->str_index = position.str_index;
    parser->str_pos = position.str_pos;
}

static bool
is_inline_start(struct cymkd_parser *parser)
{
    bool is_inline;
    int ch = next(parser);
    switch(ch) {
    case '*':
    case '_':
    case '`':
        consume(parser);
        if (next(parser) == '`') {
            is_inline = false;
        } else {
            is_inline = true;
        }
        unconsume(parser);
        break;
    case '[':
        is_inline = true;
        break;
    case '!':
        is_inline = true;
        consume(parser);
        if (next(parser) == '[') {
            is_inline = true;
        } else {
            is_inline = false;
        }
        unconsume(parser);
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
    struct cymkd_parser_position position = save_position(parser);

    success = match(parser, '*');
    if (!success) {
        success = match(parser, '_');
        if (!success) {
            restore_position(parser, position); /* Go back */
            return false;
        }
        success = match(parser, '_');
        if (!success) {
            restore_position(parser, position); /* Go back */
            return false;
        }
        start_ch = '_';
    } else {
        success = match(parser, '*');
        if (!success) {
            restore_position(parser, position); /* Go back */
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
        restore_position(parser, position); /* Go back */
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
    parser_emit_string(parser, "<code>");
    while ((ch = consume(parser)) != '`' && ch != -1) {
        parser_emit_char(parser, ch);
    }
    if (ch != '`') {
        return false;
    }
    parser_emit_string(parser, "</code>");
    return true;
}

static bool
a_link(struct cymkd_parser *parser)
{
    int ch;
    int len;
    int bufsize;
    char *link_text;
    char *link_href;

    if (!match(parser, '[')) {
        return false;
    }
    bufsize = 10;
    len = 0;
    link_text = malloc(bufsize);
    while ((ch = next(parser)) != ']') {
        if (len + 1 >= bufsize - 1) {
            bufsize *= 2;
            link_text = realloc(link_text, bufsize);
        }
        link_text[len] = ch;
        len++;
        consume(parser);
    }
    link_text[len] = '\0';
    if (!match(parser, ']')) {
        return false;
    }
    if (!match(parser, '(')) {
        return false;
    }
    bufsize = 10;
    len = 0;
    link_href = malloc(bufsize);
    while ((ch = next(parser)) != ')') {
        if (len + 1 >= bufsize - 1) {
            bufsize *= 2;
            link_href = realloc(link_href, bufsize);
        }
        link_href[len] = ch;
        len++;
        consume(parser);
    }
    link_href[len] = '\0';
    if (!match(parser, ')')) {
        return false;
    }
    parser_emit_string(parser, "<a href=\"%s\">%s</a>", link_href, link_text);
    free(link_href);
    free(link_text);
    return true;
}

static bool
img(struct cymkd_parser *parser)
{
    int ch;
    int len;
    int bufsize;
    char *alt_text;
    char *img_src;
    char img_fmt[] = "<img src=\"%s\" alt=\"%s\" />";

    if (!match(parser, '!')) {
        return false;
    }
    if (!match(parser, '[')) {
        return false;
    }
    len = 0;
    alt_text = malloc(bufsize);
    while ((ch = next(parser)) != ']') {
        if (len + 1 >= bufsize - 1) {
            bufsize *= 2;
            alt_text = realloc(alt_text, bufsize);
        }
        alt_text[len] = ch;
        len++;
        consume(parser);
    }
    alt_text[len] = '\0';
    if (!match(parser, ']')) {
        return false;
    }
    if (!match(parser, '(')) {
        return false;
    }
    bufsize = 10;
    len = 0;
    img_src = malloc(bufsize);
    while ((ch = next(parser)) != ')') {
        if (len + 1 >= bufsize - 1) {
            bufsize *= 2;
            img_src = realloc(img_src, bufsize);
        }
        img_src[len] = ch;
        len++;
        consume(parser);
    }
    img_src[len] = '\0';
    if (!match(parser, ')')) {
        return false;
    }
    parser_emit_string(parser, img_fmt, img_src,  alt_text);
    free(img_src);
    free(alt_text);
    return true;
}

static bool
inline_section(struct cymkd_parser *parser)
{
    int ch = next(parser);
    char *inline_type;
    switch (ch) {
    case '*':
    case '_':
        inline_type = "emphasis";
        break;
    case '`':
        inline_type = "code";
        break;
    case '[':
        inline_type = "link";
        break;
    case '!':
        inline_type = "img";
        break;
    default:
        inline_type = "unknown";
        break;
    }
    if (!bold(parser) 
            && !italics(parser)
            && !inline_code(parser)
            && !a_link(parser)
	    && !img(parser)) {
        char err_fmt[] = "Invalid inline section of type: %s\n";
        cymkd_error(parser, err_fmt, inline_type);
        return false;
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
            if (ch == ESCAPE_CHAR) {
                consume(parser);
                ch = next(parser);
                parser_emit_char(parser, ch);
            } else if (is_inline_start(parser)) {
                if (!inline_section(parser)) {
                    return false;
                } else if (next(parser) >= 0) {
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
            parser_emit_string(parser, "</p>");
            break;
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
            fprintf(stderr, "ERROR: Header must end with two newlines\n");
            return false;
        }
        parser_emit_string(parser, "</h%d>", header_level);
        return true;
    }
    return false;
}

static bool
unordered_list_line(struct cymkd_parser *parser)
{
    int ch;
    if (match(parser, '*') || match(parser, '-')) {
        parser_emit_string(parser, "<li>");
        while ((ch = next(parser)) != '\n' && ch != EOF) {
            parser_emit_char(parser, ch);
            consume(parser); // Move past the character
        }
        parser_emit_string(parser, "</li>");
        return true;
    }
    return false;
}

static bool
unordered_list(struct cymkd_parser *parser)
{
    if (next(parser) == '*' || next(parser) == '-') {
        parser_emit_string(parser, "<ul>");
        if (unordered_list_line(parser)) {
            while (match(parser, '\n')
                   && unordered_list_line(parser)
                   && next(parser) >= 0) {
                ; // Loop until list is finished
            }
            parser_emit_string(parser, "</ul>");
            return true;
        }
    }
    return false;
}

static bool
block_quote_line(struct cymkd_parser *parser)
{
    if (!match(parser, '>')) {
        return false;
    }
    int ch;
    while (next(parser) == ' ') { /* skip whitespace */
        consume(parser);
    }
    while ((ch = next(parser)) != '\n' && ch >= 0) {
        if (ch == ESCAPE_CHAR) {
            consume(parser);
            ch = next(parser);
        }
        parser_emit_char(parser, ch);
        consume(parser);
    }
    return true;
}

static bool
block_quote(struct cymkd_parser *parser)
{
    if (next(parser) == '>') {
        parser_emit_string(parser, "<blockquote>");
        while (block_quote_line(parser)) {
            if (!match(parser, '\n')) {
                return false;
            }
            if (next(parser) == '\n' || next(parser) < 0) {
                parser_emit_string(parser, "</blockquote>");
                return true;
            }
            parser_emit_char(parser, ' ');
        }
        return true;
    }
    return false;
}

static bool
code_line(struct cymkd_parser *parser)
{
    int ch;
    while ((ch = next(parser)) != '\n') {
        if (ch == '<') {
            parser_emit_string(parser, "&lt;");
        } else if (ch == '>') {
            parser_emit_string(parser, "&gt;");
        } else {
            parser_emit_char(parser, ch);
        }
        consume(parser);
    }
    return true;
}

static bool
code_block(struct cymkd_parser *parser)
{
    int i;
    for (i = 0; i < 3; i++) {
        if (!match(parser, '`')) {
            return false;
        }
    }
    /* Skip past the source type */
    while (next(parser) != '\n') {
        consume(parser);
    }
    if (!match(parser, '\n')) {
        return false;
    }
    parser_emit_string(parser, "<pre><code>");
    while (code_line(parser)) {
        if (!match(parser, '\n')) {
            return false;
        }
        parser_emit_char(parser, '\n');
        if (next(parser) == '`') {
            bool done = true;
            for (i = 0; i < 3; i++) {
                if (!match(parser, '`')) {
                    done = false;
                    break;
                }
            }
            if (!match(parser, '\n')) {
                return false;
            }
            if (done) {
                break;
            }
        }
    }
    parser_emit_string(parser, "</code></pre>");
    return true;
}

static bool
block(struct cymkd_parser *parser)
{
    bool success;
    success = header(parser);
    if (!success) {
        success = unordered_list(parser);
        if (!success) {
            success = block_quote(parser);
            if (!success) {
                success = code_block(parser);
                if (!success) {
                    success = paragraph(parser);
                }
            }
        }
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

static void
skip_empty_lines(struct cymkd_parser *parser)
{
    while (next(parser) == '\n') {
	    consume(parser);
    }
}

static bool 
document(struct cymkd_parser *parser)
{
    bool success;
    skip_empty_lines(parser);
    success = block(parser);
    if (success) {
        success = more_blocks(parser);
    }
    return success;
}

static void
_cymkd_render(struct cymkd_parser *parser)
{
    if (!document(parser)) {
        fprintf(stderr, "ERROR: Markdown document is invalid\n");
    }
}

void
cymkd_render(const char *file_name,
             const char *str,
             size_t str_len,
             FILE *out_fp)
{
    struct cymkd_parser parser;
    parser.str_pos = str;
    parser.str_len = str_len;
    parser.str_index = 0;
    parser.out_fp = out_fp;
    parser.out_fd = -1;
    parser.line = 1;
    parser.col = 1;
    parser.file_name = file_name;
    _cymkd_render(&parser);
}

void
cymkd_render_file(const char *file_name, FILE *in_fp, FILE *out_fp)
{
    int ch;
    char *content;
    size_t content_len;
    size_t content_bufsize;

    content_bufsize = BUFSIZE;
    content = malloc(content_bufsize);
    content_len = 0;
    while ((ch = fgetc(in_fp)) != EOF) {
        if (content_len + 1 >= content_bufsize - 1) {
            content_bufsize *= 2;
            content = realloc(content, content_bufsize); 
        }
        content[content_len] = ch;
        content_len++;
    }
    content[content_len] = '\0';
    cymkd_render(file_name, content, content_len, out_fp);
    free(content);
}

void
cymkd_render_fd(const char *file_name, int in_fd, int out_fd)
{
    struct stat statbuf;
    char *content;
    size_t content_len;

    if (in_fd == STDIN_FILENO) {
        char *msg = "ERROR: Cannot use stdin for input to cymkd_render_fd()";
        fprintf(stderr, "%s\n", msg);
        return;
    }

    fstat(in_fd, &statbuf);
    content_len = statbuf.st_size;
    content = mmap(NULL, content_len, PROT_READ, MAP_PRIVATE, in_fd, 0);
    struct cymkd_parser parser;
    parser.str_pos = content;
    parser.str_len = content_len;
    parser.str_index = 0;
    parser.out_fp = NULL;
    parser.out_fd = out_fd;
    parser.line = 1;
    parser.col = 1;
    parser.file_name = file_name;
    _cymkd_render(&parser);
    munmap(content, content_len);
}
