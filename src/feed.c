#include "cyto_config.h"
#include <ctache/ctache.h>
#include <stdio.h>
#include <time.h>
    
void
generate_feed(struct cyto_config config, ctache_data_t *posts)
{
    char feed_file_name[] = "_site/feed.xml";
    FILE *fp = fopen(feed_file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not open file: %s\n", feed_file_name);
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
    fprintf(fp, "\t<title>%s</title>\n", config.title);
    fprintf(fp, "\t<link href=\"%s\" />\n", config.url);
    fprintf(fp, "\t<updated>%s</updated>\n", now_str);
    fprintf(fp, "\t<author>\n");
    fprintf(fp, "\t\t<name>%s</name>\n", config.author);
    fprintf(fp, "\t</author>\n");
    /* TODO: Intert entries for posts */
    fprintf(fp, "</feed>");

    fclose(fp);
}
