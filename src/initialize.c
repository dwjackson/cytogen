/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2020 David Jackson
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include "common.h"
#include "initialize.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MKDIR_MODE 0770
#define LAYOUTS_DIR "_layouts"
#define INDENT "    "
#define POSTS_DIR "_posts"
#define DATE_FMT_BUFSIZE 11

static void
create_project_directory(const char *project_name)
{
    if (mkdir(project_name, MKDIR_MODE) != 0) {
        char *err_fmt = "ERROR: Could not create project directory: %s\n";
        fprintf(stderr, err_fmt, project_name);
        exit(EXIT_FAILURE);
    }
}

static void 
create_default_config_file()
{
    FILE *fp = fopen(CONFIG_FILE_NAME, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not open config file for writing\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "{\n");
    fprintf(fp, "\t\"title\": \"SITE_TITLE\",\n");
    fprintf(fp, "\t\"url\": \"SITE_URL\",\n");
    fprintf(fp, "\t\"author\": \"YOUR_NAME\"\n");
    fprintf(fp, "}");
    fclose(fp);
}

static void
create_layouts_directory()
{
    if (mkdir(LAYOUTS_DIR, MKDIR_MODE) != 0) {
        fprintf(stderr, "ERROR: Could not create _layouts directory\n");
        exit(EXIT_FAILURE);
    }
}

static void
create_default_layout()
{
    char *file_name;
    if (asprintf(&file_name, "%s/%s", LAYOUTS_DIR, "default.html") == -1) {
        fprintf(stderr, "ERROR: Could not create default layout file name\n");
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Could not create default layout\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "<!DOCTYPE html>\n");
    fprintf(fp, "<html>\n");
    fprintf(fp, "%s<head>\n", INDENT);
    fprintf(fp, "%s%s<meta charset=\"utf-8\">\n", INDENT, INDENT);
    fprintf(fp, "%s%s<title>SITE_TITLE</title>\n", INDENT, INDENT);
    fprintf(fp, "%s</head>\n", INDENT);
    fprintf(fp, "%s<body>\n", INDENT);
    fprintf(fp, "%s%s<nav><a href=\"/\">Home</a></nav>\n", INDENT, INDENT);
    fprintf(fp, "%s%s{{>content}}\n", INDENT, INDENT);
    fprintf(fp, "%s</body>\n", INDENT);
    fprintf(fp, "</html>\n");

    fclose(fp);
    free(file_name);
}

static void
create_posts_directory()
{
    if (mkdir(POSTS_DIR, MKDIR_MODE) != 0) {
        fprintf(stderr, "ERROR: Could not create _posts directory\n");
        exit(EXIT_FAILURE);
    }

    time_t now;
    time(&now);
    struct tm tm;
    localtime_r(&now, &tm);

    char today[DATE_FMT_BUFSIZE];
    strftime(today, DATE_FMT_BUFSIZE, "%Y-%m-%d", &tm);

    char *file_path;
    asprintf(&file_path, "%s/%s-example-post.md", POSTS_DIR, today);

    FILE *fp = fopen(file_path, "w");
    if (fp == NULL) {
        char err_fmt[] = "ERROR: Could not open file for writing: %s\n";
        fprintf(stderr, err_fmt, file_path);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "---\n");
    fprintf(fp, "layout: default\n");
    fprintf(fp, "title: Example Post\n");
    fprintf(fp, "---\n\n");
    fprintf(fp, "This is an *example* post.");

    fclose(fp);
    free(file_path);
}

static bool
file_exists(const char *file_name)
{
    struct stat statbuf;
    if (stat(file_name, &statbuf) != -1) {
        return true;
    }
    return false;
}

static void 
create_index_page()
{
    char file_name[] = "index.html";

    if (file_exists(file_name)) {
        return;
    }

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: could not open for writing: %s", file_name);
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "---\nlayout: default\n---\n\n");
    fprintf(fp, "<h2>Posts</h2>\n\n");
    fprintf(fp, "<ul>\n");
    fprintf(fp, "{{#posts}}\n");
    fprintf(fp, "%s<li>", INDENT);
    fprintf(fp, "<a href=\"{{url}}\">{{title}} ({{date}})</a>");
    fprintf(fp, "</li>\n");
    fprintf(fp, "{{/posts}}\n");
    fprintf(fp, "</ul>\n");
    fclose(fp);
}
    
void
cmd_initialize(const char *project_name)
{
    if (strcmp(project_name, CURRENT_DIRECTORY) != 0) {
        create_project_directory(project_name);
        chdir(project_name);
    }

    create_default_config_file();
    create_layouts_directory();
    create_default_layout();
    create_posts_directory();
    create_index_page();
}
