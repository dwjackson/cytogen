/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef CYTO_HEADER_H
#define CYTO_HEADER_H

#include <stdio.h>
#include <ctache/ctache.h>

int
cytogen_header_read_from_file(FILE *fp, ctache_data_t *data);

int
cytogen_header_read_from_string(const char *str, ctache_data_t *data);

#endif /* CYTO_HEADER_H */
