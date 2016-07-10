#include "cymkd.h"
#include <stdio.h>
#include <stdlib.h>

#define USAGE_FMT "Usage: %s [markdown_file]"
#define CONTENTS_DEFAULT_BUFSIZE 1024

int
main(int argc, char *argv[])
{
    char *file_name;
    FILE *in_fp;
    FILE *out_fp = stdout;
    char *contents;
    size_t contents_len;
    size_t contents_bufsize;
    int ch;

    if (argc != 2 && argc != 1) {
        printf(USAGE_FMT, argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 1) {
        in_fp = stdin;
    } else {
        file_name = argv[1];
        in_fp = fopen(file_name, "r");
        if (in_fp == NULL) {
            printf("ERROR: Could not open file: %s\n", file_name);
            exit(EXIT_FAILURE);
        }
    }

    contents_bufsize = CONTENTS_DEFAULT_BUFSIZE;
    contents = malloc(contents_bufsize + 1);
    while ((ch = fgetc(in_fp)) != EOF) {
        if (contents_len + 1 >= contents_bufsize) {
            contents_bufsize *= 2;
            contents = realloc(contents, contents_bufsize + 1);
        }
        contents[contents_len] = ch;
        contents_len++;
    }
    contents[contents_len] = '\0';
    cymkd_render(contents, contents_len, out_fp);
    free(contents);

    if (in_fp != stdin) {
        fclose(in_fp);
    }

    return 0;
}
