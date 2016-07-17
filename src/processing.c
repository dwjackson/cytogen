/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "layout.h"
#include "processing.h"
#include "files.h"
#include "string_util.h"
#include "cytoplasm_header.h"
#include "cymkd.h"
#include <pthread.h>
#include <ctache/ctache.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>

#define LAYOUT "layout"

char
*determine_out_file_name(const char *in_file_name,
                         const char *site_dir,
                         pthread_mutex_t *basename_mtx)
{
    char *in_file_name_dup;
    char *out_file_name;
    size_t out_file_name_len;

    in_file_name_dup = strdup(in_file_name);
    pthread_mutex_lock(basename_mtx);
    char *in_file_base_name = basename(in_file_name_dup);
    pthread_mutex_unlock(basename_mtx);

    out_file_name_len = strlen(site_dir) + 1 + strlen(in_file_base_name);
    out_file_name = malloc(out_file_name_len + 1);
    strcpy(out_file_name, site_dir);
    strcat(out_file_name, "/");
    strcat(out_file_name, in_file_base_name);
    if (out_file_name == NULL) {
        fprintf(stderr, "ERROR: Could not malloc() for out_file_name\n");
        return NULL;
    }

    free(in_file_name_dup);

    return out_file_name;
}

static void
render_markdown(const char *file_name)
{
    FILE *in_fp = fopen(file_name, "r");
    if (in_fp == NULL) {
        fprintf(stderr, "ERROR: Could not open for reading: %s", file_name);
        return;
    }

    char *html_file_name;
    asprintf(&html_file_name, "%s.html", file_name);
    FILE *out_fp = fopen(html_file_name, "w");
    if (out_fp == NULL) {
        char *err_fmt = "ERROR: Could not open for writing: %s\n";
        fprintf(stderr, err_fmt, html_file_name);
        perror(NULL);
    }
    cymkd_render_file(in_fp, out_fp);
    fclose(in_fp);
    fclose(out_fp);
}

void
process_file(const char *in_file_name,
             struct process_file_args *args,
             ctache_data_t *file_data)
{
    char *in_file_extension;
    char *out_file_name;
    const char *site_dir = args->site_dir;

    in_file_extension = file_extension(in_file_name);
    out_file_name = determine_out_file_name(in_file_name,
                                            site_dir,
                                            args->basename_mutex);

    bool is_markdown = false;
    is_markdown = extension_implies_markdown(in_file_extension);
    
    FILE *in_fp = fopen(in_file_name, "r");
    if (in_fp != NULL) {
        cytoplasm_header_read(in_fp, file_data);
        FILE *out_fp = fopen(out_file_name, "w");
        if (out_fp != NULL) {
            if (!ctache_data_hash_table_has_key(file_data, LAYOUT)) {
                /* Render the file as a ctache template */
                ctache_render_file(in_fp, out_fp, file_data, ESCAPE_HTML);
            } else {
                /*
                 * Render the layout with the file content passed as a
                 * partial with the key "content"
                 */
                char *content = read_file_contents(in_fp);
                size_t content_len = strlen(content);
                ctache_data_t *content_data;
                content_data = ctache_data_create_string(content,
                                                         content_len);
                ctache_data_hash_table_set(file_data,
                                           "content",
                                           content_data);
                ctache_data_t *layout_data;
                layout_data = ctache_data_hash_table_get(file_data, LAYOUT);
                char *layout_name = string_trim(layout_data->data.string);
                char *layout = get_layout_content(args->layouts,
                                                  args->num_layouts,
                                                  layout_name);
                if (layout == NULL) {
                    char *err_fmt = "ERROR: Layout not found: \"%s\"\n";
                    fprintf(stderr, err_fmt, layout_name);
                    abort();
                }
                size_t layout_len = strlen(layout);
                ctache_render_string(layout,
                                     layout_len,
                                     out_fp,
                                     file_data,
                                     ESCAPE_HTML);
                free(layout_name);
            }

            /* Clean up the output file pointer */
            fclose(out_fp);
            out_fp = NULL;
        } else {
            char *err_fmt = "ERROR: Could not open output file: %s\n";
            fprintf(stderr, err_fmt, out_file_name);
        }
        fclose(in_fp);

        /* If necessary render the output file as markdown */
        if (is_markdown) {
            render_markdown(out_file_name);
            unlink(out_file_name);
        }

        pthread_mutex_lock(args->data_mutex);
        ctache_data_hash_table_set(args->data, in_file_name, file_data);
        pthread_mutex_unlock(args->data_mutex);
    } else {
        char *err_fmt = "ERROR: Could not open input file %s\n";
        fprintf(stderr, err_fmt, in_file_name);
    }
    free(out_file_name);
    free(in_file_extension);
}

void
*process_files(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    int i;
    char *in_file_name;
    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        ctache_data_t *file_data = ctache_data_create_hash();
        process_file(in_file_name, args, file_data);
    }

    return NULL;
}

void
*process_post_files(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    ctache_data_t *posts_arr = ctache_data_hash_table_get(args->data, "posts");
    int i;
    char *in_file_name;
    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        ctache_data_t *file_data = ctache_data_create_hash();
        process_file(in_file_name, args, file_data);
        ctache_data_t *post_data = ctache_data_create_hash();
        // TODO: Fill in posts data to posts array
        ctache_data_array_append(posts_arr, post_data);
    }
    return NULL;
}
