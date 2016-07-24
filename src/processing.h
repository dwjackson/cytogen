/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef PROCESSING_H
#define PROCESSING_H

#include "layout.h"
#include <pthread.h>
#include <ctache/ctache.h>

struct process_file_args {
    int start_index;
    int end_index;
    char **file_names;
    ctache_data_t *data;
    pthread_mutex_t *data_mutex;
    pthread_mutex_t *basename_mutex;
    struct layout *layouts;
    int num_layouts;
    const char *site_dir;
};

char
*determine_out_file_name(const char *in_file_name,
                         const char *site_dir,
                         pthread_mutex_t *basename_mtx);

void
process_file(const char *in_file_name,
             struct process_file_args *args,
             ctache_data_t *file_data);

void
*process_files(void *args_ptr);

void
*process_post_files(void *args_ptr);

#endif /* PROCESSING_H */
