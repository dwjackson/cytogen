/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cymkd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define USAGE_FMT "Usage: %s [markdown_file]\n"
#define CONTENTS_DEFAULT_BUFSIZE 1024
#define INDENT "    "

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
    int opt;
    bool no_wrap = false;
    char *out_file_name = NULL;

    extern char *optarg;
    extern int optind;

    while ((opt = getopt(argc, argv, "no:")) != -1) {
        switch (opt) {
        case 'n':
            no_wrap = true;
            break;
        case 'o':
            out_file_name = strdup(optarg);
            break;
        default:
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (optind >= argc) {
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

    if (out_file_name != NULL) {
        out_fp = fopen(out_file_name, "w");
        if (out_fp == NULL) {
            char *err_fmt = "ERROR: Could not open for writing: %s\n";
            fprintf(stderr, err_fmt, out_file_name);
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

    if (!no_wrap) {
        fprintf(out_fp, "<!DOCTYPE html>\n");
        fprintf(out_fp, "<html>\n");
        fprintf(out_fp, "%s<head>\n", INDENT);
        fprintf(out_fp, "%s%s<meta charset=\"utf-8\">\n", INDENT, INDENT);
        fprintf(out_fp, "%s</head>\n", INDENT);
        fprintf(out_fp, "%s<body>\n", INDENT);
    }

    cymkd_render(contents, contents_len, out_fp);

    if (!no_wrap) {
        fprintf(out_fp, "\n%s</body>\n", INDENT);
        fprintf(out_fp, "</html>");
    }

    /* Add a final newline */
    fprintf(out_fp, "\n");

    /* Cleanup */
    free(contents);
    if (in_fp != stdin) {
        fclose(in_fp);
    }
    if (out_fp != stdout) {
        fclose(out_fp);
    }
    if (out_file_name != NULL) {
        free(out_file_name);
    }

    return 0;
}
