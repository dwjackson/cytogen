/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cytogen_header.h"
#include "string_util.h"
#include "files.h"

#define CYTO_HEADER_BORDER "---"

static char *reserved_words[] = {
    "content",
    "posts"
};
#define NUM_RESERVED_WORDS 1

static bool
is_reserved(const char *str)
{
    bool reserved = false;
    int i;
    char *word;
    for (i = 0; i < NUM_RESERVED_WORDS; i++) {
        word = reserved_words[i];
        if (strcmp(str, word) == 0) {
            reserved = true;
            break;
        }
    }
    return reserved;
}

static char
*read_line_from_file(FILE *fp)
{
    int ch; /* Character/byte */

    /* Get the line length */
    char *line;
    int line_length = 0;
    while ((ch = fgetc(fp)) != EOF && ch != '\n') {
        line_length++;
    }
    fseek(fp, -1 * line_length - 1, SEEK_CUR); /* Rewind */

    /* Read the line */
    line = malloc(line_length + 1);
    memset(line, 0, line_length + 1);
    int i;
    for (i = 0; i < line_length; i++) {
        ch = fgetc(fp);
        line[i] = ch;
    }
    fgetc(fp); /* Throw away the newline/EOF */

    return line;
}

int
next_line_length(const char *str, size_t str_len, int start)
{
    int line_len;
    int ch;
    int idx;

    if (start >= str_len) {
        return -1;
    }

    line_len = 0;
    idx = start;
    while ((ch = str[idx++]) != '\0' && ch != '\n') {
        line_len++;
    }
    return line_len;
}

static char
*read_line_from_string(const char *str, size_t str_len, int start)
{
    char *line;
    int ch;
    int line_len = next_line_length(str, str_len, start);
    if (line_len < 0) {
        return NULL;
    }
    line = malloc(line_len + 1);
    if (line == NULL) {
        fprintf(stderr, "ERROR: Could not malloc in read_line_from_string()\n");
        abort();
    }
    memset(line, 0, line_len + 1);
    strncpy(line, str + start, line_len);
    return line;
}

int
cytogen_header_read_from_string(const char *str, ctache_data_t *data)
{
    size_t str_len;
    int str_index;
    char *line;
    size_t line_len;
    int ch;
    char *key;
    char *value;
    int i;
    int index;
    int header_length;

    str_len = strlen(str);
    key = NULL;
    value = NULL;
    str_index = 0;
    line = read_line_from_string(str, str_len, str_index);
    if (line == NULL) {
        return 0; /* No line could be read */
    }
    line_len = strlen(line);
    str_index += line_len + 1; /* The +1 is for the newline */

    if (strcmp(line, CYTO_HEADER_BORDER) == 0) {
        line = read_line_from_string(str, str_len, str_index);
        line_len = strlen(line);
        str_index += line_len + 1; /* The +1 is for the newline */

        while (strcmp(line, CYTO_HEADER_BORDER) != 0 && str_index < str_len) {
            if (line_len == 0) {
                /* Skip empty lines */
                line = read_line_from_string(str, str_len, str_index);
                line_len = strlen(line);
                str_index += 1; /* Add 1 to skip newline */
                continue;
            }
            key = malloc(line_len + 1);
            memset(key, 0, line_len + 1);
            value = malloc(line_len + 1);
            memset(value, 0, line_len + 1);
            for (i = 0; i < line_len; i++) {
                ch = line[i];
                if (ch == ':') {
                    index = i + 1;
                    break;
                }
                key[i] = ch;
            }
            for (i = index; i < line_len; i++) {
                ch = line[i];
                value[i - index] = ch;
            }

            if (data != NULL
                    && !ctache_data_hash_table_has_key(data, key)
                    && !is_reserved(key)) {
                ctache_data_t *str_data;
                char *value_trimmed = string_trim(value);
                size_t value_len = strlen(value_trimmed);
                str_data = ctache_data_create_string(value_trimmed, value_len);
                ctache_data_hash_table_set(data, key, str_data);
                free(value_trimmed);
                free(value);
            }

            value = NULL;
            free(key);
            key = NULL;
            free(line);

            line = read_line_from_string(str, str_len, str_index);
            line_len = strlen(line);
            str_index += line_len + 1; /* The +1 is for the newline */
        }
        header_length = str_index;
    } else {
        header_length = 0;
    }

    if (key != NULL) {
        free(key);
    }
    free(line);

    return header_length;
}

int
cytogen_header_read_from_file(FILE *fp, ctache_data_t *data)
{
    char *file_content = read_file_contents(fp);
    fseek(fp, 0, SEEK_SET); /* Rewind the file */

    int header_length = cytogen_header_read_from_string(file_content, data);
    bool file_has_header = header_length > 0;
    if (file_has_header) {
        fseek(fp, header_length, SEEK_CUR); /* Fast-forward to end of header */
    }

    free(file_content);

    return header_length;
}
