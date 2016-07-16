/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef CYTOPLASM_STRING_UTIL
#define CYTOPLASM_STRING_UTIL

#include <stdbool.h>
#include <stdarg.h>

char
*string_trim(const char *str);

bool
string_matches_any(const char *str, int num_possible_matches, ...);

#endif /* CYTOPLASM_STRING_UTIL */
