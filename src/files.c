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
#include <sys/stat.h>
#include <stdbool.h>

#define DEFAULT_CONTENT_LENGTH 1024
    
void
get_file_list(const char *dir_name,
              char ***file_names_ptr,
              int *num_files_ptr, 
              char ***directories_ptr,
              int *num_directories_ptr)
{
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        fprintf(stderr, "ERROR: Could not open current directory\n");
        exit(EXIT_FAILURE);
    }

    int num_files = 0;
    int num_directories = 0;
    struct dirent *de;
    struct stat statbuf;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        char *file_path = malloc(strlen(dir_name) + 1 + strlen(file_name) + 1);
        strcpy(file_path, dir_name);
        strcat(file_path, "/");
        strcat(file_path, file_name);
        if (stat(file_path, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            num_files++;
        } else if (S_ISDIR(statbuf.st_mode)
                   && file_name[0] != '_'
                   && file_name[0] != '.') {
            num_directories++;
        }
        free(file_path);
    }
    *num_files_ptr = num_files;
    *num_directories_ptr = num_directories;
    rewinddir(dir);

    char **file_names = malloc(sizeof(char*) * num_files);
    char **directory_names = malloc(sizeof(char*) * num_directories);
    int index = 0;
    int dir_index = 0;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        char *file_path = malloc(strlen(dir_name) + 1 + strlen(file_name) + 1);
        strcpy(file_path, dir_name);
        strcat(file_path, "/");
        strcat(file_path, file_name);
        if (stat(file_path, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            file_names[index] = strdup(file_path);
            index++;
        } else if (S_ISDIR(statbuf.st_mode)
                   && file_name[0] != '_'
                   && file_name[0] != '.') {
            directory_names[dir_index] = strdup(file_name);
            dir_index++;
        }
        free(file_path);
    }

    closedir(dir);
    *file_names_ptr = file_names;
    *directories_ptr = directory_names;
}

char
*file_extension(const char *file_name)
{
    size_t file_name_len;
    int i;
    char ch;
    size_t extension_len;
    char *extension;
    int start_index;

    file_name_len = strlen(file_name);
    extension_len = 0;
    for (i = file_name_len - 1; i >= 0; i--) {
        ch = file_name[i];
        if (ch != '.') {
            extension_len++;
        } else {
            start_index = i + 1;
            break;
        }
    }

    extension = malloc(extension_len + 1);
    memset(extension, 0, extension_len + 1);
    for (i = start_index; i < file_name_len; i++) {
        ch = file_name[i];
        extension[i - start_index] = ch;
    }

    return extension;
}

char
*read_file_contents(FILE *fp)
{
    char *content;
    size_t content_length;
    size_t content_bufsize;
    int ch;

    content_bufsize = DEFAULT_CONTENT_LENGTH ;
    content = malloc(content_bufsize);
    content_length = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (content_length + 1 >= content_bufsize) {
            content_bufsize *= 2;
            content = realloc(content, content_bufsize);
        }
        content[content_length] = ch;
        content_length++;
    }
    content[content_length] = '\0';
    return content;
}

bool
extension_implies_markdown(const char *extension)
{
    return (strcmp(extension, "md") == 0 || strcmp(extension, "mkd") == 0);
}
