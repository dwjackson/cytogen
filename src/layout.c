/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2021 David Jackson
 */

#include "config.h"

#include "layout.h"
#include "cytogen_header.h"
#include <ctache/ctache.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define LAYOUTS_DIR_NAME "_layouts"

static int
layouts_count(DIR *layouts_dir)
{
    struct dirent *de;
    int num_layouts = 0;
    while ((de = readdir(layouts_dir)) != NULL) {
        char *file_name = de->d_name;
        if (file_name[0] != '.') {
            num_layouts++;
        }
    }
    rewinddir(layouts_dir);
    return num_layouts;
}

static char
*layout_name_from_file_name(const char *file_name, size_t file_name_len)
{
    char *layout_name = malloc(file_name_len + 1);
    if (layout_name == NULL) {
        fprintf(stderr, "ERROR: Could not malloc() for layout_name\n");
        abort();
    }
    memset(layout_name, 0, file_name_len);
    int i;
    int ch;
    for (i = 0; i < file_name_len; i++) {
        ch = file_name[i];
        if (ch == '.') {
            break;
        }
        layout_name[i] = ch;
    }
    return layout_name;
}

static char
*layout_content_read(int fd, size_t file_size)
{
    size_t content_len = file_size + 1;
    char *content = malloc(content_len);
    memset(content, 0, content_len);
    ssize_t bytes_read = read(fd, content, file_size);
    if (bytes_read == -1) {
        fprintf(stderr, "ERROR: Could not read layout content\n");
        abort();
    }
    return content;
}

static void
layout_content_destroy(struct layout layout)
{
    free(layout.content);
}

static void
read_layout_from_file(const char *file_name, struct layout *layout)
{
    size_t file_name_len = strlen(file_name);
    char *layout_name = layout_name_from_file_name(file_name,
                                                   file_name_len);

    char *file_path;
    asprintf(&file_path, "%s/%s", LAYOUTS_DIR_NAME, file_name);

    struct stat statbuf;
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: Could not open %s\n", file_path);
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, &statbuf) < 0) {
        fprintf(stderr, "ERROR: Could not stat %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    size_t size = statbuf.st_size;
    char *content = layout_content_read(fd, size);

    struct layout l = { layout_name, content, size };
    *layout = l;

    close(fd);
    free(file_path);
}

struct layout
*get_layouts(int *num_layouts_ptr)
{
    struct layout *layouts = NULL;
    int num_layouts = 0;

    struct dirent *de;
    DIR *layouts_dir = opendir(LAYOUTS_DIR_NAME);
    if (layouts_dir != NULL) {
        num_layouts = layouts_count(layouts_dir);
        
        layouts = malloc(sizeof(struct layout) * num_layouts);
        if (layouts == NULL) {
            fprintf(stderr, "ERROR: Could not malloc() for layouts\n");
            abort();
        }

        int index = 0;
        while ((de = readdir(layouts_dir)) != NULL) {
            char *file_name = de->d_name;
            if (file_name[0] != '.') {
                struct layout layout;
                read_layout_from_file(file_name, &layout);
                layouts[index] = layout;
                index++;
            }
        }
        closedir(layouts_dir);
    }

    /* Render any recursive layouts */
    int i;
    for (i = 0; i < num_layouts; i++) {
        struct layout layout = layouts[i];
        char *content = layout.content;
        ctache_data_t *header_data = ctache_data_create_hash();
        int header_len = cytogen_header_read_from_string(content, header_data);
        int repetitions = 0;
        while (header_len > 0) {
            /*
             * Fail if we do more than num_layouts-squared iterations because
             * this algorithm should be polynomial and if we do more than this
             * something has gone wrong.
             */
            if (repetitions > num_layouts * num_layouts) {
                fprintf(stderr, "Layouts overflowed maximum iterations");
                abort();
            }

            content += header_len; /* Skip past the header */
            char *out;
            if (ctache_data_hash_table_has_key(header_data, "layout")) {
                ctache_data_t *str_data;
                str_data = ctache_data_hash_table_get(header_data, "layout");
                const char *layout_name = ctache_data_string_buffer(str_data);
                char *layout_content = get_layout_content(layouts,
                                                          num_layouts,
                                                          layout_name);
                content = layout_content;
            }
            char *partial_pos = strstr(content, "{{>content}}");
            char *chptr = content;
            int ch;
            int out_len = 0;
            int out_bufsize= 10;
            out = malloc(out_bufsize);
            while ((ch = *chptr) != '\0') {
                if (chptr != partial_pos) {
                    if (out_len + 1 >= out_bufsize - 1) { /* -1 for '\0' */
                        out_bufsize *= 2;
                        out = realloc(out, out_bufsize);
                    }
                    out[out_len] = ch;
                    out_len++;
                } else {
                    chptr += strlen("{{>content}}");
                    char *layout_content = layout.content + header_len;
                    while ((ch = *layout_content++) != '\0') {
                        if (out_len + 1 >= out_bufsize - 1) { /* -1 for '\0' */
                            out_bufsize *= 2;
                            out = realloc(out, out_bufsize);
                        }
                        out[out_len] = ch;
                        out_len++;
                    }
                }
                chptr++;
            }
            out[out_len] = '\0';
            content = out;
            ctache_data_destroy(header_data);
            header_data = ctache_data_create_hash();
            header_len = cytogen_header_read_from_string(content, header_data);

            repetitions++;
        }
        ctache_data_destroy(header_data);
        layouts[i].content = content;
    }

    *num_layouts_ptr = num_layouts;
    return layouts;
}

void
layouts_destroy(struct layout *layouts, int num_layouts)
{
    int i;
    struct layout layout;
    for (i = 0; i < num_layouts; i++) {
        layout = layouts[i];
        free(layout.name);
        layout_content_destroy(layout);
    }
}

char
*get_layout_content(struct layout *layouts, int num_layouts, const char *name)
{
    char *layout_content = NULL;
    int i;
    struct layout layout;
    for (i = 0; i < num_layouts; i++) {
        layout = layouts[i];
        if (strcmp(layout.name, name) == 0) {
            layout_content = layout.content;
            break;
        }
    }
    return layout_content;
}
