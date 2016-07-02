#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
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
                // TODO: create the layout (use mmap for file content, shared)
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
    for (i = 0; i < num_layouts; i++) {
        // TODO
    }
}
