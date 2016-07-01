#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define USAGE "Usage: cyto [COMMAND]"

#define NUM_WORKERS 4
#define SITE_DIR "_site"

struct layout {
    char *name;
    char *content;
};

static struct layout
*get_layouts(int *num_layouts_ptr)
{
    struct layout *layouts = NULL;
    int num_layouts = 0;

    struct dirent *de;
    DIR *layouts_dir = opendir("_layouts");
    if (layouts_dir != NULL) {
        while ((de = readdir(layouts_dir)) != NULL) {
            char *file_name = de->d_name;
            if (file_name[0] != '.') {
                num_layouts++;
            }
        }
        rewinddir(layouts_dir);

        layouts = malloc(sizeof(struct layout) * num_layouts);
        if (layouts == NULL) {
            fprintf(stderr, "ERROR: Could not malloc()\n");
            abort();
        }

        int index = 0;
        while ((de = readdir(layouts_dir)) != NULL) {
            char *file_name = de->d_name;
            if (file_name[0] != '.') {
                // TODO: create the layout (use mmap for file content, shared)
                index++;
            }
        }
        closedir(layouts_dir);
    }
    *num_layouts_ptr = num_layouts;
    return layouts;
}

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

static void
cmd_generate()
{
    mkdir(SITE_DIR, 0770);

    int num_files;
    char **file_names = get_file_list(&num_files);
    int num_layouts;
    struct layout *layouts = get_layouts(&num_layouts);
    pid_t child_pids[NUM_WORKERS];
    int i;
    pid_t pid;

    /* Create worker processes */
    for (i = 0; i < NUM_WORKERS && pid != 0; i++) {
        pid = fork();
        if (pid > 0) {
            child_pids[i] = pid;
        }
    }

    if (pid == 0) {
        // TODO: Do work as worker/child process
        exit(EXIT_SUCCESS);
    }
    
    /* Wait for worker processes to finish */
    for (i = 0; i < NUM_WORKERS; i++) {
        int stat_loc;
        waitpid(child_pids[i], &stat_loc, 0);
    }
    free(file_names);
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
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", cmd);
        exit(EXIT_FAILURE);
    }

    return 0;
}
