/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2017-2021 David Jackson
 */

#include "config.h"

#include "generate.h"
#include "processing.h"
#include "files.h"
#include "layout.h"
#include <string.h>
#include <sys/stat.h>

/*
 * Used to sort file names of posts in reverse-alphanumeric order so that the
 * files sort most-recent to least-recent by date.
 */
static int
file_name_compare(const void *file_name_1, const void *file_name_2)
{
    const char **fname1_ptr = (const char **) file_name_1;
    const char **fname2_ptr = (const char **) file_name_2;
    int strcmp_retval = strcmp(*fname1_ptr, *fname2_ptr);
    return strcmp_retval * -1;
}

/* Set up the threads to process files, process them, tear down threads */
static void
_generate(int num_workers,
          const char *site_dir,
          int num_files,
          char **file_names,
          struct layout *layouts,
          int num_layouts,
          ctache_data_t *data,
          pthread_mutex_t *data_mutex,
          void *(*process)(void*))
{
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
        threads_args[i].data_mutex = data_mutex;
        threads_args[i].layouts = layouts;
        threads_args[i].num_layouts = num_layouts;
        threads_args[i].site_dir = site_dir;
        pthread_create(&(thr_pool[i]), NULL, process, &(threads_args[i]));
    }

    /* Wait for workers to finish */
    for (i = 0; i < num_workers; i++) {
        pthread_join(thr_pool[i], NULL);
    }

    /* Threads Cleanup */
    free(threads_args);
    free(thr_pool);
}
    
void
generate(struct generate_arguments *args)
{
    const char *site_dir;
    const char *curr_dir_name;
    char **file_names;
    int num_files;
    char **directories;
    int num_directories;

    get_file_list(args->curr_dir_name,
                  &file_names,
                  &num_files,
                  &directories,
                  &num_directories);
    qsort(file_names, num_files, sizeof(char *), file_name_compare);
    int num_layouts;
    struct layout *layouts = get_layouts(&num_layouts);

    site_dir = args->site_dir;
    curr_dir_name = args->curr_dir_name;

    mkdir(site_dir, 0770);

    _generate(args->num_workers,
              site_dir,
              num_files,
              file_names,
              layouts,
              num_layouts,
              args->data,
              args->data_mutex,
              args->process);

    /* Process the subdirectories, recursively */
    int i;
    for (i = 0; i < num_directories; i++) {
        char *directory = directories[i];

        char *subdir;
        asprintf(&subdir, "%s/%s", curr_dir_name, directory);

        char *site_subdir;
        asprintf(&site_subdir, "%s/%s", site_dir, directory);

        struct generate_arguments args_r; /* Recursive call args */
        args_r = *args;
        args_r.curr_dir_name = subdir;
        args_r.site_dir = site_subdir;
        generate(&args_r);

        free(site_subdir);
        free(subdir);
    }
    
    /* Final Cleanup */
    layouts_destroy(layouts, num_layouts);
    for (i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);
    free(directories);
}
