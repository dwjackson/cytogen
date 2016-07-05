#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ctache/ctache.h>
#include "layout.h"
#include "cytoplasm_header.h"

#define USAGE "Usage: cyto [COMMAND]"

#define NUM_WORKERS 4
#define SITE_DIR "_site"
#define DEFAULT_CONTENT_LENGTH 1024
#define LAYOUT "layout"

static char
**get_file_list(int *num_files_ptr)
{
    DIR *dir = opendir(".");
    if (dir == NULL) {
        fprintf(stderr, "ERROR: Could not open current directory\n");
        exit(EXIT_FAILURE);
    }

    int num_files = 0;
    struct dirent *de;
    struct stat statbuf;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        if (stat(file_name, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            num_files++;
        }
    }
    *num_files_ptr = num_files;
    rewinddir(dir);

    char **file_names = malloc(sizeof(char*) * num_files);
    int index = 0;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        if (stat(file_name, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            file_names[index] = file_name;
            index++;
        }
    }

    closedir(dir);

    return file_names;
}

static char
*read_file_contents(FILE *fp)
{
    char *content;
    size_t content_length;
    size_t content_bufsize;
    char ch;

    content_bufsize = DEFAULT_CONTENT_LENGTH ;
    content = malloc(content_bufsize);
    content_length = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (content_length + 1 >= content_bufsize) {
            content_bufsize *= 2;
            content = realloc(content, content_bufsize);
        }
        content[content_length] = ch;
        content_length++;
    }
    content[content_length] = '\0';
    return content;
}

struct process_file_args {
    int start_index;
    int end_index;
    char **file_names;
    ctache_data_t *data;
    pthread_mutex_t *data_mutex;
    struct layout *layouts;
    int num_layouts;
};

static void
*process_file(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    int i;
    char *in_file_name;
    char *out_file_name;
    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        out_file_name = malloc(strlen(SITE_DIR) + 1 + strlen(in_file_name) + 1);
        strcpy(out_file_name, SITE_DIR);
        strcat(out_file_name, "/");
        strcat(out_file_name, in_file_name);
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
                    ctache_data_hash_table_set(file_data, "content", content);
                    ctache_data_t *layout_data;
                    layout_data = ctache_data_hash_table_get(file_data, LAYOUT);
                    char *layout_name = layout_data->data.string;
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
                    free(content);
                }

                /* Clean up the file pointer */
                fclose(out_fp);
                out_fp = NULL;
            } else {
                fprintf(stderr, "ERROR: Could not open %s\n", out_file_name);
            }
            fclose(in_fp);
            pthread_mutex_lock(args->data_mutex);
            ctache_data_hash_table_set(args->data, in_file_name, file_data);
            pthread_mutex_unlock(args->data_mutex);
        } else {
            fprintf(stderr, "ERROR: Could not open %s\n", in_file_name);
        }
        free(out_file_name);
    }

    return NULL;
}

static void
cmd_generate()
{
    mkdir(SITE_DIR, 0770);

    int num_files;
    char **file_names = get_file_list(&num_files);
    int num_layouts;
    struct layout *layouts = get_layouts(&num_layouts);

    /* Set up the data */
    ctache_data_t *data = ctache_data_create_hash();
    pthread_mutex_t data_mutex;
    pthread_mutex_init(&data_mutex, NULL);

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
        threads_args[i].layouts = layouts;
        threads_args[i].num_layouts = num_layouts;
        pthread_create(&(thr_pool[i]), NULL, process_file, &(threads_args[i]));
    }

    /* Wait for workers to finish */
    for (i = 0; i < NUM_WORKERS; i++) {
        pthread_join(thr_pool[i], NULL);
    }

    /* Cleanup */
    pthread_mutex_destroy(&data_mutex);
    ctache_data_destroy(data);
    layouts_destroy(layouts, num_layouts);
    free(file_names);
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
        cmd_generate();
    } else if (strcmp(cmd, "init") == 0) {
        if (argc == 3) {
            char *proj_name = argv[2];
            cmd_initialize(proj_name);
        } else {
            fprintf(stderr, "ERROR: No project name given\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}
