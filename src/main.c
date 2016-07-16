/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

/* Linux's feature-test macros can break other systems, turn on all features */
#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include "layout.h"
#include "cytoplasm_header.h"
#include "cymkd.h"
#include "files.h"
#include "processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ctache/ctache.h>
#include <ftw.h>

#define USAGE "Usage: cyto [COMMAND]"

#define NUM_WORKERS 4
#define SITE_DIR "_site"

static void
_generate(const char *curr_dir_name, const char *site_dir)
{
    mkdir(site_dir, 0770);

    char **file_names;
    int num_files;
    char **directories;
    int num_directories;
    get_file_list(curr_dir_name,
                  &file_names,
                  &num_files,
                  &directories,
                  &num_directories);
    int num_layouts;
    struct layout *layouts = get_layouts(&num_layouts);

    /* Set up the data */
    ctache_data_t *data = ctache_data_create_hash();
    pthread_mutex_t data_mutex;
    pthread_mutex_init(&data_mutex, NULL);

    /* Set up the basename(3) mutex */
    pthread_mutex_t basename_mutex;
    pthread_mutex_init(&basename_mutex, NULL);

    int files_per_worker = num_files / NUM_WORKERS;
    pthread_t thr_pool[NUM_WORKERS];
    struct process_file_args threads_args[NUM_WORKERS];

    /* Create workers */
    int i;
    for (i = 0; i < NUM_WORKERS; i++) {
        threads_args[i].start_index = i * files_per_worker;
        if (i + 1 < NUM_WORKERS) {
            threads_args[i].end_index = threads_args[i].start_index
                + files_per_worker;
        } else {
            threads_args[i].end_index = num_files;
        }
        threads_args[i].file_names = file_names;
        threads_args[i].data = data;
        threads_args[i].data_mutex = &data_mutex;
        threads_args[i].basename_mutex = &basename_mutex;
        threads_args[i].layouts = layouts;
        threads_args[i].num_layouts = num_layouts;
        threads_args[i].site_dir = site_dir;
        pthread_create(&(thr_pool[i]), NULL, process_files, &(threads_args[i]));
    }

    /* Wait for workers to finish */
    for (i = 0; i < NUM_WORKERS; i++) {
        pthread_join(thr_pool[i], NULL);
    }

    /* Process the subdirectories, recursively */
    for (i = 0; i < num_directories; i++) {
        char *directory = directories[i];

        size_t subdir_len = strlen(curr_dir_name) + 1 + strlen(directory);
        char *subdir = malloc(subdir_len + 1);
        strcpy(subdir, curr_dir_name);
        strcat(subdir, "/");
        strcat(subdir, directory);

        size_t site_subdir_len = strlen(site_dir) + 1 + strlen(directory);
        char *site_subdir = malloc(site_subdir_len + 1);
        strcpy(site_subdir, site_dir);
        strcat(site_subdir, "/");
        strcat(site_subdir, directory);
        _generate(subdir, site_subdir);

        free(site_subdir);
        free(subdir);
    }

    /* Cleanup */
    pthread_mutex_destroy(&data_mutex);
    pthread_mutex_destroy(&basename_mutex);
    ctache_data_destroy(data);
    layouts_destroy(layouts, num_layouts);
    for (i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);
    free(directories);
}

static void
cmd_generate(const char *curr_dir_name, const char *site_dir)
{
    _generate(curr_dir_name, site_dir);
}

static void
cmd_initialize(const char *project_name)
{
    if (mkdir(project_name, 0770) != 0) {
        char *err_fmt = "ERROR: Could not create project directory: %s\n";
        fprintf(stderr, err_fmt, project_name);
    }
}

int
_clean(const char *path, const struct stat *stat, int flag, struct FTW *ftw_ptr)
{
    if (flag == FTW_DP) {
        rmdir(path);
    } else if (flag == FTW_F) {
        unlink(path);
    } else {
        fprintf(stderr, "Unrecognized file type for %s\n", path);
        return -1;
    }
    return 0;
}

void
cmd_clean()
{
    nftw(SITE_DIR, _clean, 1000, FTW_DEPTH); 
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s\n", USAGE);
        exit(EXIT_FAILURE);
    }

    char *cmd = argv[1];
    if (string_matches_any(cmd, 3, "g", "gen", "generate")) {
        cmd_generate(".", SITE_DIR);
    } else if (string_matches_any(cmd, 1, "init")) {
        if (argc == 3) {
            char *proj_name = argv[2];
            cmd_initialize(proj_name);
        } else {
            fprintf(stderr, "ERROR: No project name given\n");
            exit(EXIT_FAILURE);
        }
    } else if (string_matches_any(cmd, 1, "clean")) { 
        cmd_clean();
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}
