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
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "layout.h"

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
    void *region;
    region = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    char *content = (char *)region;
    return content;
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
                size_t file_name_len = strlen(file_name);
                char *layout_name = layout_name_from_file_name(file_name,
                                                               file_name_len);
                char *file_path = malloc(strlen(LAYOUTS_DIR_NAME) + 1
                                         + file_name_len
                                         + 1);
                strcpy(file_path, LAYOUTS_DIR_NAME);
                strcat(file_path, "/");
                strcat(file_path, file_name);
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
                layouts[index] = l;
                close(fd);
                free(file_path);
                index++;
            }
        }
        closedir(layouts_dir);
    }
    *num_layouts_ptr = num_layouts;
    return layouts;
}

static void
layout_content_destroy(struct layout layout)
{
    munmap(layout.content, layout.length);
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
