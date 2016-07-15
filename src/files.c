#include <stdio.h>
#include <stdlib.h>
#include <strlen.h>
#include <dirent.h>
#include <sys/stat.h>
    
void
get_file_list(const char *dir_name,
              char ***file_names_ptr,
              int *num_files_ptr, 
              char ***directories_ptr,
              int *num_directories_ptr)
{
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        fprintf(stderr, "ERROR: Could not open current directory\n");
        exit(EXIT_FAILURE);
    }

    int num_files = 0;
    int num_directories = 0;
    struct dirent *de;
    struct stat statbuf;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        char *file_path = malloc(strlen(dir_name) + 1 + strlen(file_name) + 1);
        strcpy(file_path, dir_name);
        strcat(file_path, "/");
        strcat(file_path, file_name);
        if (stat(file_path, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            num_files++;
        } else if (S_ISDIR(statbuf.st_mode)
                   && file_name[0] != '_'
                   && file_name[0] != '.') {
            num_directories++;
        }
        free(file_path);
    }
    *num_files_ptr = num_files;
    *num_directories_ptr = num_directories;
    rewinddir(dir);

    char **file_names = malloc(sizeof(char*) * num_files);
    char **directory_names = malloc(sizeof(char*) * num_directories);
    int index = 0;
    int dir_index = 0;
    while ((de = readdir(dir)) != NULL) {
        char *file_name = de->d_name;
        char *file_path = malloc(strlen(dir_name) + 1 + strlen(file_name) + 1);
        strcpy(file_path, dir_name);
        strcat(file_path, "/");
        strcat(file_path, file_name);
        if (stat(file_path, &statbuf) == -1) {
            fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
            exit(EXIT_FAILURE);
        }
        if (!S_ISDIR(statbuf.st_mode)
            && file_name[0] != '_'
            && file_name[0] != '.') {
            file_names[index] = strdup(file_path);
            index++;
        } else if (S_ISDIR(statbuf.st_mode)
                   && file_name[0] != '_'
                   && file_name[0] != '.') {
            directory_names[dir_index] = strdup(file_name);
            dir_index++;
        }
        free(file_path);
    }

    closedir(dir);
    *file_names_ptr = file_names;
    *directories_ptr = directory_names;
}
