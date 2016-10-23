/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifndef FEED_H
#define FEED_H

#include "cyto_config.h"
#include <ctache/ctache.h>

void
generate_feed(struct cyto_config *config, ctache_data_t *posts);

#endif /* FEED_H */
