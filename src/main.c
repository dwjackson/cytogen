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
#include "string_util.h"
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

#define DEFAULT_NUM_WORKERS 4
#define SITE_DIR "_site"
#define POSTS_DIR "_posts"

struct generate_arguments {
    const char *curr_dir_name;
    const char *site_dir;
    int num_workers;
};

static void
_generate(struct generate_arguments *args)
{
    const char *site_dir;
    const char *curr_dir_name;
    char **file_names;
    int num_files;
    char **directories;
    int num_directories;

    site_dir = args->site_dir;
    curr_dir_name = args->curr_dir_name;

    mkdir(site_dir, 0770);

    get_file_list(args->curr_dir_name,
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

    /* Set up the posts data */
    struct stat statbuf;
    if (stat(POSTS_DIR, &statbuf) == 0 && statbuf.st_mode & S_IFDIR) {
        ctache_data_t *posts_array = ctache_data_create_array(0);
        ctache_data_hash_table_set(data, "posts", posts_array);
    }

    /* Set up the basename(3) mutex */
    pthread_mutex_t basename_mutex;
    pthread_mutex_init(&basename_mutex, NULL);

    int num_workers = args->num_workers;
    int files_per_worker = num_files / num_workers;
    pthread_t *thr_pool = malloc(sizeof(pthread_t) * num_workers);
    size_t arr_size = sizeof(struct process_file_args) * num_workers;
    struct process_file_args *threads_args = malloc(arr_size);

    /* Create workers */
    int i;
    for (i = 0; i < num_workers; i++) {
        threads_args[i].start_index = i * files_per_worker;
        if (i + 1 < num_workers) {
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
    for (i = 0; i < num_workers; i++) {
        pthread_join(thr_pool[i], NULL);
    }

    /* Threads Cleanup */
    free(threads_args);
    free(thr_pool);
    pthread_mutex_destroy(&data_mutex);
    pthread_mutex_destroy(&basename_mutex);

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

        struct generate_arguments args_r; /* Recursive call args */
        args_r.curr_dir_name = subdir;
        args_r.site_dir = site_subdir;
        args_r.num_workers = num_workers;
        _generate(&args_r);

        free(site_subdir);
        free(subdir);
    }
    
    /* Final Cleanup */
    ctache_data_destroy(data);
    layouts_destroy(layouts, num_layouts);
    for (i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);
    free(directories);
}

static void
cmd_generate(const char *curr_dir_name, const char *site_dir, int num_workers)
{
    struct generate_arguments args;
    args.curr_dir_name = curr_dir_name;
    args.site_dir = site_dir;
    args.num_workers = num_workers;
    _generate(&args);
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
    int num_workers;
    char **args;

    int opt;
    extern char *optarg;
    extern int optind;
    while ((opt = getopt(argc, argv, "w:")) != -1) {
        switch (opt) {
        case 'w':
            num_workers = atoi(optarg);
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    args = argv + optind;

    if (num_workers < 1) {
        num_workers = DEFAULT_NUM_WORKERS;
    }

    char *cmd = args[0];
    if (string_matches_any(cmd, 3, "g", "gen", "generate")) {
        cmd_generate(".", SITE_DIR, num_workers);
    } else if (string_matches_any(cmd, 1, "init")) {
        if (argc == 3) {
            char *proj_name = args[1];
            cmd_initialize(proj_name);
        } else {
            fprintf(stderr, "ERROR: No project name given\n");
            exit(EXIT_FAILURE);
        }
    } else if (string_matches_any(cmd, 2, "c", "clean")) { 
        cmd_clean();
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}
