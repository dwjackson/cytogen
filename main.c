#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include "layout.h"

#define USAGE "Usage: cyto [COMMAND]"

#define NUM_WORKERS 4
#define SITE_DIR "_site"

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
