/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2021 David Jackson
 */

#ifndef CYTO_CONFIG_H
#define CYTO_CONFIG_H

#include "cyjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctache/ctache.h>

struct cyto_config {
    char *title;
    char *url;
    char *author;
};

int
cyto_config_read(const char *file_name, struct cyto_config *config);

void
cyto_config_destroy(struct cyto_config *config);

ctache_data_t
*cyto_config_to_ctache_data(struct cyto_config *config);

#endif /* CYTO_CONFIG_H */
