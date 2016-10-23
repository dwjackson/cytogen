/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cyto_config.h"
#include <ctache/ctache.h>
#include <stdio.h>
#include <time.h>

static void
insert_entries(FILE *fp, ctache_data_t *posts)
{
    size_t posts_length;
    ctache_data_t *post;
    ctache_data_t *str_data;
    int i;

    posts_length = ctache_data_length(posts);
    for (i = 0; i < posts_length; i++) {
        post = ctache_data_array_get(posts, i);
        fprintf(fp, "\t<entry>\n");
        str_data = ctache_data_hash_table_get(post, "title");
        fprintf(fp, "\t\t<title>%s</title>\n", str_data->data.string);
        str_data = ctache_data_hash_table_get(post, "url");
        fprintf(fp, "\t\t<link href=\"%s\" />\n", str_data->data.string);
        fprintf(fp, "\t\t<id>%s</id>\n", str_data->data.string);
        fprintf(fp, "\t</entry>\n");
    }
}
    
void
generate_feed(struct cyto_config *config, ctache_data_t *posts)
{
    char feed_file_name[] = "_site/feed.xml";
    FILE *fp = fopen(feed_file_name, "w");
    if (fp == NULL) {
        char fmt[] = "ERROR: Could not open file: %s\n";
        fprintf(stderr, fmt, feed_file_name);
        perror(NULL);
        return;
    }

    time_t now;
    struct tm tm;
    char now_str[30];
    time(&now);
    localtime_r(&now, &tm);
    strftime(now_str, 30, "%Y-%m-%dT%H:%M:%SZ", &tm);

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(fp, "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n");
    fprintf(fp, "\t<title>%s</title>\n", config->title);
    fprintf(fp, "\t<link href=\"%s\" />\n", config->url);
    fprintf(fp, "\t<updated>%s</updated>\n", now_str);
    fprintf(fp, "\t<id>%s</id>\n", config->url);
    fprintf(fp, "\t<author>\n");
    fprintf(fp, "\t\t<name>%s</name>\n", config->author);
    fprintf(fp, "\t</author>\n");

    insert_entries(fp, posts);

    fprintf(fp, "</feed>");

    fclose(fp);
}
