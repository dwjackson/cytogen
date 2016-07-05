/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef LAYOUT_H
#define LAYOUT_H

struct layout {
    char *name;
    char *content;
    size_t length;
};

struct layout
*get_layouts(int *num_layouts_ptr);

void
layouts_destroy(struct layout *layouts, int num_layouts);

char
*get_layout_content(struct layout *layouts, int num_layouts, const char *name);

#endif /* LAYOUT_H */
