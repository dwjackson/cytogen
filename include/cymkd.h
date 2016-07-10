/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef CYMKD_H
#define CYMKD_H

#include <stdio.h>
#include <stdlib.h>

void
cymkd_parse(const char *str, size_t str_len, FILE *out_fp);

#endif /* CYMKD_H */
