/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif /* __linux__ */

#include "layout.h"
#include "cytoplasm_header.h"
#include "string_util.h"
#include "cymkd.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ctache/ctache.h>
#include <libgen.h>
#include <ftw.h>
#include <sys/mman.h>
#include <fcntl.h>

#define USAGE "Usage: cyto [COMMAND]"

#define NUM_WORKERS 4
#define SITE_DIR "_site"
#define LAYOUT "layout"

struct process_file_args {
    int start_index;
    int end_index;
    char **file_names;
    ctache_data_t *data;
    pthread_mutex_t *data_mutex;
    pthread_mutex_t *basename_mutex;
    struct layout *layouts;
    int num_layouts;
    const char *site_dir;
};

static void
*process_files(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    int i;
    const char *site_dir = args->site_dir;
    char *in_file_name;
    char *in_file_extension;
    char *out_file_name;
    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        in_file_extension = file_extension(in_file_name);

        char *in_file_name_dup = strdup(in_file_name);
        pthread_mutex_lock(args->basename_mutex);
        char *in_file_base_name = basename(in_file_name_dup);
        pthread_mutex_unlock(args->basename_mutex);
        size_t out_file_name_len;
        out_file_name_len = strlen(site_dir) + 1 + strlen(in_file_base_name);
        out_file_name = malloc(out_file_name_len + 1);
        strcpy(out_file_name, site_dir);
        strcat(out_file_name, "/");
        strcat(out_file_name, in_file_base_name);
        if (out_file_name == NULL) {
            fprintf(stderr, "ERROR: Could not malloc()\n");
            break;
        }

        FILE *in_fp = fopen(in_file_name, "r");
        if (in_fp != NULL) {
            ctache_data_t *file_data = ctache_data_create_hash();
            cytoplasm_header_read(in_fp, file_data);
            FILE *out_fp = fopen(out_file_name, "w");
            if (out_fp != NULL) {
                if (!ctache_data_hash_table_has_key(file_data, LAYOUT)) {
                    /* Render the file as a ctache template */
                    ctache_render_file(in_fp, out_fp, file_data, ESCAPE_HTML);
                } else {
                    /*
                     * Render the layout with the file content passed as a
                     * partial with the key "content"
                     */
                    char *content = read_file_contents(in_fp);
                    size_t content_len = strlen(content);
                    ctache_data_t *content_data;
                    content_data = ctache_data_create_string(content,
                                                             content_len);
                    ctache_data_hash_table_set(file_data,
                                               "content",
                                               content_data);
                    ctache_data_t *layout_data;
                    layout_data = ctache_data_hash_table_get(file_data, LAYOUT);
                    char *layout_name = string_trim(layout_data->data.string);
                    char *layout = get_layout_content(args->layouts,
                                                      args->num_layouts,
                                                      layout_name);
                    if (layout == NULL) {
                        char *err_fmt = "ERROR: Layout not found: \"%s\"\n";
                        fprintf(stderr, err_fmt, layout_name);
                        abort();
                    }
                    size_t layout_len = strlen(layout);
                    ctache_render_string(layout,
                                         layout_len,
                                         out_fp,
                                         file_data,
                                         ESCAPE_HTML);
                    free(layout_name);
                }

                /* Clean up the output file pointer */
                fclose(out_fp);
                out_fp = NULL;
            } else {
                char *err_fmt = "ERROR: Could not open output file: %s\n";
                fprintf(stderr, err_fmt, out_file_name);
            }
            fclose(in_fp);

            /* If necessary render the output file as markdown */
            if (extension_implies_markdown(in_file_extension)) {
                int in_fd = open(out_file_name, O_RDONLY);
                char template[] = "/tmp/cymkd.XXXXXX";
                int tmp_fd = mkstemp(template);
                cymkd_render_fd(in_fd, tmp_fd);
                close(in_fd);
                lseek(tmp_fd, 0, SEEK_SET); /* Rewind the output file */
                struct stat statbuf;
                fstat(tmp_fd, &statbuf);
                size_t content_len = statbuf.st_size;
                char *content = mmap(NULL,
                                     content_len,
                                     PROT_READ,
                                     MAP_PRIVATE,
                                     tmp_fd,
                                     0);
                char *html_file_name;
                asprintf(&html_file_name, "%s.html", out_file_name);
                FILE *fp = fopen(html_file_name, "w");
                if (fp == NULL) {
                    char *err_fmt = "ERROR: Could not open output file: %s\n";
                    fprintf(stderr, err_fmt, out_file_name);
                } else {
                    int i;
                    int ch;
                    for (i = 0; i < content_len; i++) {
                        ch = content[i];
                        fputc(ch, fp);
                    }
                }
                unlink(out_file_name);
                free(html_file_name);
                munmap(content, content_len);
                close(tmp_fd);
            }

            pthread_mutex_lock(args->data_mutex);
            ctache_data_hash_table_set(args->data, in_file_name, file_data);
            pthread_mutex_unlock(args->data_mutex);
        } else {
            char *err_fmt = "ERROR: Could not open input file %s\n";
            fprintf(stderr, err_fmt, in_file_name);
        }
        free(out_file_name);
        free(in_file_extension);
        free(in_file_name_dup);
    }

    return NULL;
}

static void
_generate(const char *curr_dir_name, const char *site_dir)
{
    mkdir(site_dir, 0770);

    char **file_names;
    int num_files;
    char **directories;
    int num_directories;
    get_file_list(curr_dir_name,
                  &file_names,
                  &num_files,
                  &directories,
                  &num_directories);
    int num_layouts;
    struct layout *layouts = get_layouts(&num_layouts);

    /* Set up the data */
    ctache_data_t *data = ctache_data_create_hash();
    pthread_mutex_t data_mutex;
    pthread_mutex_init(&data_mutex, NULL);

    /* Set up the basename(3) mutex */
    pthread_mutex_t basename_mutex;
    pthread_mutex_init(&basename_mutex, NULL);

    int files_per_worker = num_files / NUM_WORKERS;
    pthread_t thr_pool[NUM_WORKERS];
    struct process_file_args threads_args[NUM_WORKERS];

    /* Create workers */
    int i;
    for (i = 0; i < NUM_WORKERS; i++) {
        threads_args[i].start_index = i * files_per_worker;
        if (i + 1 < NUM_WORKERS) {
            threads_args[i].end_index = threads_args[i].start_index
                + files_per_worker;
        } else {
            threads_args[i].end_index = num_files;
        }
        threads_args[i].file_names = file_names;
        threads_args[i].data = data;
        threads_args[i].data_mutex = &data_mutex;
        threads_args[i].basename_mutex = &basename_mutex;
        threads_args[i].layouts = layouts;
        threads_args[i].num_layouts = num_layouts;
        threads_args[i].site_dir = site_dir;
        pthread_create(&(thr_pool[i]), NULL, process_files, &(threads_args[i]));
    }

    /* Wait for workers to finish */
    for (i = 0; i < NUM_WORKERS; i++) {
        pthread_join(thr_pool[i], NULL);
    }

    /* Process the subdirectories, recursively */
    for (i = 0; i < num_directories; i++) {
        char *directory = directories[i];

        size_t subdir_len = strlen(curr_dir_name) + 1 + strlen(directory);
        char *subdir = malloc(subdir_len + 1);
        strcpy(subdir, curr_dir_name);
        strcat(subdir, "/");
        strcat(subdir, directory);

        size_t site_subdir_len = strlen(site_dir) + 1 + strlen(directory);
        char *site_subdir = malloc(site_subdir_len + 1);
        strcpy(site_subdir, site_dir);
        strcat(site_subdir, "/");
        strcat(site_subdir, directory);
        _generate(subdir, site_subdir);

        free(site_subdir);
        free(subdir);
    }

    /* Cleanup */
    pthread_mutex_destroy(&data_mutex);
    pthread_mutex_destroy(&basename_mutex);
    ctache_data_destroy(data);
    layouts_destroy(layouts, num_layouts);
    for (i = 0; i < num_files; i++) {
        free(file_names[i]);
    }
    free(file_names);
    free(directories);
}

static void
cmd_generate(const char *curr_dir_name, const char *site_dir)
{
    _generate(curr_dir_name, site_dir);
}

static void
cmd_initialize(const char *project_name)
{
    if (mkdir(project_name, 0770) != 0) {
        char *err_fmt = "ERROR: Could not create project directory: %s\n";
        fprintf(stderr, err_fmt, project_name);
    }
}

int
_clean(const char *path, const struct stat *stat, int flag, struct FTW *ftw_ptr)
{
    if (flag == FTW_DP) {
        rmdir(path);
    } else if (flag == FTW_F) {
        unlink(path);
    } else {
        fprintf(stderr, "Unrecognized file type for %s\n", path);
        return -1;
    }
    return 0;
}

void
cmd_clean()
{
    nftw(SITE_DIR, _clean, 1000, FTW_DEPTH); 
}
    
int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s\n", USAGE);
        exit(EXIT_FAILURE);
    }

    char *cmd = argv[1];
    if (strcmp(cmd, "g") == 0
        || strcmp(cmd, "gen") == 0
        || strcmp(cmd, "generate") == 0) {
        cmd_generate(".", SITE_DIR);
    } else if (strcmp(cmd, "init") == 0) {
        if (argc == 3) {
            char *proj_name = argv[2];
            cmd_initialize(proj_name);
        } else {
            fprintf(stderr, "ERROR: No project name given\n");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(cmd, "clean") == 0) {
        cmd_clean();
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}
