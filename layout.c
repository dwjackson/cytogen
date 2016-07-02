#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "layout.h"
    
struct layout
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
                size_t file_name_len = strlen(file_name);
                char *layout_name = malloc(file_name_len + 1);
                memset(layout_name, 0, file_name_len);
                int i;
                int ch;
                for (i = 0; i < file_name_len; i++) {
                    ch = file_name[i];
                    if (ch == '.') {
                        break;
                    }
                    layout_name[i] = ch;
                }
                struct stat statbuf; 
                if (stat(file_name, &statbuf) < 0) {
                    fprintf(stderr, "ERROR: Could not stat %s\n", file_name);
                    exit(EXIT_FAILURE);
                }
                int fd = open(file_name, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "ERROR: Could not open %s\n", file_name);
                    exit(EXIT_FAILURE);
                }
                void *r; /* Region */
                size_t size = statbuf.st_size;
                r = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
                char *content = (char *)r;
                struct layout l = { layout_name, content, size };
                layouts[index] = l;
                close(fd);
                index++;
            }
        }
        closedir(layouts_dir);
    }
    *num_layouts_ptr = num_layouts;
    return layouts;
}

void
layouts_destroy(struct layout *layouts, int num_layouts)
{
    int i;
    struct layout layout;
    for (i = 0; i < num_layouts; i++) {
        layout = layouts[i];
        free(layout.name);
        munmap(layout.content, layout.length);
    }
}

char
*get_layout_content(struct layout *layouts, int num_layouts, const char *name)
{
    char *layout_content = NULL;
    int i;
    struct layout layout;
    for (i = 0; i < num_layouts; i++) {
        layout = layouts[i];
        if (strcmp(layout.name, name) == 0) {
            layout_content = layout.content;
            break;
        }
    }
    return layout_content;
}
