/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdbool.h>

void
get_file_list(const char *dir_name,
              char ***file_names_ptr,
              int *num_files_ptr, 
              char ***directories_ptr,
              int *num_directories_ptr);

char
*file_extension(const char *file_name);

char
*read_file_contents(FILE *fp);

bool
extension_implies_markdown(const char *extension);

#endif /* FILES_H */
