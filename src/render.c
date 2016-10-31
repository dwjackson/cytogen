/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "common.h"
#include "files.h"
#include "layout.h"
#include "string_util.h"
#include "cymkd.h"
#include <stdio.h>
#include <string.h>
#include <ctache/ctache.h>

#define DELIM_BEGIN "{{"
#define DELIM_END "}}"

/*
 * Render the layout with the file content passed as a partial with the key
 * "content".
 */
static void
render_with_layout(FILE *in_fp,
                   FILE *out_fp,
                   struct layout *layouts,
                   int num_layouts,
                   ctache_data_t *file_data)
{
    char *content = read_file_contents(in_fp);
    size_t content_len = strlen(content);
    ctache_data_t *content_data;
    content_data = ctache_data_create_string(content, content_len);
    ctache_data_hash_table_set(file_data, "content", content_data);
    ctache_data_t *layout_data = ctache_data_hash_table_get(file_data, LAYOUT);
    char *layout_name = string_trim(layout_data->data.string);
    char *layout = get_layout_content(layouts, num_layouts, layout_name);
    if (layout == NULL) {
        fprintf(stderr, "ERROR: Layout not found: \"%s\"\n", layout_name);
        abort();
    }
    size_t layout_len = strlen(layout);
    ctache_render_string(layout,
                         layout_len,
                         out_fp,
                         file_data,
                         ESCAPE_HTML,
                         DELIM_BEGIN,
                         DELIM_END);
    free(content);
    free(layout_name);
}
    
void
render_ctache_file(FILE *in_fp,
                   FILE* out_fp,
                   struct layout *layouts,
                   int num_layouts,
                   ctache_data_t *file_data)
{
    if (!ctache_data_hash_table_has_key(file_data, LAYOUT)) {
        ctache_render_file(in_fp,
                           out_fp,
                           file_data,
                           ESCAPE_HTML,
                           DELIM_BEGIN,
                           DELIM_END);
    } else {
        render_with_layout(in_fp, out_fp, layouts, num_layouts, file_data);
    }
}

void
render_markdown(FILE *in_fp, const char *file_name, char **html_file_name_ptr)
{
    asprintf(html_file_name_ptr, "%s.html", file_name);
    char *html_file_name = *html_file_name_ptr;
    FILE *out_fp = fopen(html_file_name, "w");
    if (out_fp == NULL) {
        char *err_fmt = "ERROR: Could not open for writing: %s\n";
        fprintf(stderr, err_fmt, html_file_name);
        perror(NULL);
    }
    cymkd_render_file(file_name, in_fp, out_fp);
    fclose(out_fp);
}
