/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef GENERATE_H
#define GENERATE_H

#include <ctache/ctache.h>
#include <pthread.h>

struct generate_arguments {
    const char *curr_dir_name;
    const char *site_dir;
    int num_workers;
    ctache_data_t *data;
    pthread_mutex_t *data_mutex;
    void *(*process)(void*);
};

void
generate(struct generate_arguments *args);

#endif /* GENERATE_H */
