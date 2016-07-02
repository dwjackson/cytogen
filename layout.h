#ifndef LAYOUT_H
#define LAYOUT_H

struct layout {
    char *name;
    char *content;
};

struct layout
*get_layouts(int *num_layouts_ptr);

void
layouts_destroy(struct layout *layouts, int num_layouts);

char
*get_layout_content(struct layouts *layouts, int num_layouts, const char *name);

#endif /* LAYOUT_H */
