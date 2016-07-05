/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "string_util.h"

static bool
is_whitespace(char ch)
{
    return (ch == ' ' || ch == '\r' || ch == '\n');
}

char
*string_trim(const char *str)
{
    char *trimmed_str;
    size_t trimmed_len;
    size_t str_len;
    size_t preceding_whitespace_len;
    size_t trailing_whitespace_len;
    char ch;
    int index;
    int begin_index;
    int i;

    str_len = strlen(str);
    index = 0;
    preceding_whitespace_len = 0;
    ch = str[0];
    while (is_whitespace(ch) && index < str_len) {
        index++;
        preceding_whitespace_len++;
        ch = str[index];
    }
    begin_index = index;

    trailing_whitespace_len = 0;
    index = str_len - 1;
    ch = str[index];
    while (is_whitespace(ch) && index >= 0) {
        index--;
        trailing_whitespace_len++;
        ch = str[index];
    }
    
    if (preceding_whitespace_len + trailing_whitespace_len >= str_len) {
        return NULL;
    }
    trimmed_len = str_len - preceding_whitespace_len - trailing_whitespace_len;
    trimmed_str = malloc(trimmed_len + 1);
    memset(trimmed_str, 0, trimmed_len + 1);
    for (i = begin_index; i < begin_index + trimmed_len; i++) {
        ch = str[i];
        trimmed_str[i - begin_index] = ch;
    }
    return trimmed_str;
}
