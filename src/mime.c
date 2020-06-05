/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2020 David Jackson
 */

#include "mime.h"
#include <string.h>

struct mime_entry {
	char *mime_type;
	char *extension;
};

static struct mime_entry mime_types[] = {
	{ .mime_type = "text/css charset=utf-8", .extension = "css" },
	{ .mime_type = "application/xml charset=utf-8", .extension = "xml" },
	{ .mime_type = "text/javascript charset=utf-8", .extension = "js" },
	{ .mime_type = "image/png", .extension = "png" },
	{ .mime_type = "image/svg+xml", .extension = "svg" },
	{ .mime_type = "image/jpeg", .extension = "jpeg" },
	{ .mime_type = "image/jpeg", .extension = "jpg" }
};

static const char
*extension(const char *file_name)
{
	size_t len = strlen(file_name);
	const char *chptr = file_name + len - 1;
	while (chptr != file_name && *chptr != '.') {
		chptr--;
	}
	if (*chptr == '.') {
		 /* Move past the '.' */
		chptr++;
	}
	return chptr;
}

char
*mime_type_of(const char *file_name)
{
	int i;
	const char *ext;
	struct mime_entry *entry;
	for (i = 0; i < sizeof(mime_types) / sizeof(struct mime_entry); i++) {
		entry = &mime_types[i];
		ext = extension(file_name);
		if (strcmp(entry->extension, ext) == 0) {
			return entry->mime_type;
		}
	}
	return NULL;
}
