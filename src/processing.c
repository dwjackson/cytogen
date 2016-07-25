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
#include <time.h>
#include <sys/param.h>
#ifdef HAVE_BASENAME_R
#include <libgen.h>
#endif /* HAVE_BASENAME_R */

#define LAYOUT "layout"

char
*determine_out_file_name(const char *in_file_name,
                         const char *site_dir)
{
    char *in_file_name_dup;
    char *out_file_name;
    size_t out_file_name_len;
    char in_file_base_name[MAXPATHLEN];

    in_file_name_dup = strdup(in_file_name);
    basename_r(in_file_name_dup, in_file_base_name);

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
    cymkd_render_file(in_fp, out_fp);
    fclose(out_fp);
}

static void
render_ctache_file(FILE *in_fp,
                   const char *out_file_name,
                   struct layout *layouts,
                   int num_layouts,
                   ctache_data_t *file_data)
{
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
            char *layout = get_layout_content(layouts,
                                              num_layouts,
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
            free(content);
            free(layout_name);
        }

        /* Clean up the output file pointer */
        fclose(out_fp);
        out_fp = NULL;
    } else {
        char *err_fmt = "ERROR: Could not open output file: %s\n";
        fprintf(stderr, err_fmt, out_file_name);
    }
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
                                            site_dir);

    bool is_markdown = false;
    is_markdown = extension_implies_markdown(in_file_extension);
    
    const char *ctache_file_name = in_file_name;
    FILE *in_fp = fopen(ctache_file_name, "r");
    if (in_fp != NULL) {
        cytoplasm_header_read(in_fp, file_data);

        /* If necessary render the output file as markdown */
        char *html_file_name = NULL;
        if (is_markdown) {
            char *markdown_file_name = out_file_name;
            render_markdown(in_fp, markdown_file_name, &html_file_name);

            fclose(in_fp);
            unlink(out_file_name);

            in_fp = fopen(html_file_name, "r");
        }

        render_ctache_file(in_fp,
                           out_file_name,
                           args->layouts,
                           args->num_layouts,
                           file_data);

        /* Clean up files created by the markdown rendering */
        if (is_markdown) {
            unlink(html_file_name);
            rename(out_file_name, html_file_name);
        }

        free(html_file_name);
        fclose(in_fp); 
    } else {
        char *err_fmt = "ERROR: Could not open input file %s\n";
        fprintf(stderr, err_fmt, ctache_file_name);
    }

    /* Final cleanup */
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
        ctache_data_t *empty = ctache_data_create_hash();
        ctache_data_t *file_data = ctache_data_merge_hashes(args->data, empty);
        process_file(in_file_name, args, file_data);
        ctache_data_destroy(file_data);
        ctache_data_destroy(empty);
    }
    return NULL;
}

time_t
date_from_file_name(const char *file_name)
{
    time_t date;
    struct tm date_tm;
    char file_name_base[MAXPATHLEN];
    char *file_name_date_portion;

    basename_r(file_name, file_name_base);
    file_name_date_portion = strdup(file_name_base);
    file_name_date_portion[10] = '\0'; /* First 10 chars are YYYY-MM-DD */
    strptime(file_name_date_portion, "%Y-%m-%d", &date_tm);
    date = mktime(&date_tm);

    free(file_name_date_portion);

    return date;
}

char
*post_url(const char *file_name)
{
    char *url;
    time_t date;
    char file_name_base[MAXPATHLEN];
    char *file_name_without_date;
    char *file_name_dup;
    char url_date[11]; /* YYYY/mm/dd */
    struct tm date_tm;

    date = date_from_file_name(file_name);
    localtime_r(&date, &date_tm);

    file_name_dup = strdup(file_name);
    /* file_name_base is the file name after the date */
    basename_r(file_name_dup, file_name_base);
    file_name_without_date = file_name_base + strlen("YYYY-MM-DD");

    char parent_dir[] = "/posts/YYYY/mm/dd/";
    url = malloc(strlen(parent_dir) + strlen(file_name_without_date) + 1);
    strftime(url_date, 10, "%Y/%m/%d", &date_tm);
    strcpy(url, "/posts/");
    strcat(url, url_date);
    strcat(url, "/");
    strcat(url, file_name_without_date);

    free(file_name_dup);

    return url;
}

static void
get_post_directory(const char *post_file_name, char *post_dir)
{
    size_t datestamp_len = strlen("YYYY-MM-DD-"); /* Assumes trailing '-' */
    const char *post = post_file_name + datestamp_len;
    int extension_start = -1;
    size_t post_file_name_len = strlen(post_file_name);
    int ch;
    int i;
    for (i = post_file_name_len - 1; i >= 0; i--) {
        ch = post_file_name[i];
        if (ch == '.') {
            extension_start = i;
            break;
        }
    }
    if (extension_start < 0) {
        extension_start = post_file_name_len;
    }
    for (i = datestamp_len; i < extension_start; i++) {
        *post_dir++ = post_file_name[i];
    }
    *post_dir = '\0';
}

static char
*prepare_post_directory(const char *site_dir,
                        const char *post_file_name)
{
    time_t date;
    struct tm date_tm;
    int year, month, day;
    struct stat statbuf;
    char *dir;

    date = date_from_file_name(post_file_name);
    localtime_r(&date, &date_tm);
    year = date_tm.tm_year + 1900;
    month = date_tm.tm_mon + 1;
    day = date_tm.tm_mday;

    /* Create the base directory if it doesn't exist */
    asprintf(&dir, "%s/posts", site_dir);
    mkdir(dir, 0770);
    free(dir);

    /* Create the year directory if it doesn't exist */
    asprintf(&dir, "%s/posts/%4d", site_dir, year);
    mkdir(dir, 0770);
    free(dir);

    /* Create the month directory if it doesn't exist */
    asprintf(&dir, "%s/posts/%4d/%02d", site_dir, year, month);
    mkdir(dir, 0770);
    free(dir);

    /* Create the day directory if it doesn't exist */
    asprintf(&dir, "%s/posts/%4d/%02d/%02d", site_dir, year, month, day);
    mkdir(dir, 0770);
    free(dir);

    /* Create the post directory if it doesn't exist */
    char *file_name_dup = strdup(post_file_name);
    char post[MAXPATHLEN];
    char file_name_base[MAXPATHLEN];
    basename_r(file_name_dup, file_name_base);
    get_post_directory(file_name_base, post);
    char fmt[] = "%s/posts/%4d/%02d/%02d/%s";
    asprintf(&dir, fmt, site_dir, year, month, day, post);
    mkdir(dir, 0770);
    free(file_name_dup);

    return dir;
}

void
*process_post_files(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    ctache_data_t *posts_arr = ctache_data_hash_table_get(args->data, "posts");
    int i;
    char *in_file_name;
    time_t date;
    char *url;
    ctache_data_t *tmp_data;

    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        ctache_data_t *file_data = ctache_data_create_hash();

        const char *site_dir = args->site_dir;
        char *post_dir = prepare_post_directory(site_dir,
                                                in_file_name);
        args->site_dir = post_dir;
        process_file(in_file_name, args, file_data);
        args->site_dir = site_dir;

        ctache_data_t *post_data = ctache_data_create_hash();
        if (!ctache_data_hash_table_has_key(file_data, "title")) {
            fprintf(stderr, "ERROR: Post has no title: %s\n", in_file_name);
        }

        /* Post Title */
        tmp_data = ctache_data_hash_table_get(file_data, "title");
        ctache_data_hash_table_set(post_data, "title", tmp_data);
        date = date_from_file_name(in_file_name);

        /* Post Creation Date */
        tmp_data = ctache_data_create_time(date);
        ctache_data_hash_table_set(post_data, "date", tmp_data);

        /* Post URL */
        url = post_url(in_file_name);
        tmp_data = ctache_data_create_string(url, strlen(url));
        ctache_data_hash_table_set(post_data, "url", tmp_data);

        /* Add the post data to the posts array */
        ctache_data_array_append(posts_arr, post_data);

        free(url);
        ctache_data_destroy(file_data);
    }
    return NULL;
}
