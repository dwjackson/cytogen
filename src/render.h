#ifndef RENDER_H
#define RENDER_H

#include <stdio.h>
#include "layout.h"
#include <ctache/ctache.h>

void
render_ctache_file(FILE *in_fp,
                   FILE* out_fp,
                   struct layout *layouts,
                   int num_layouts,
                   ctache_data_t *file_data);

void
render_markdown(FILE *in_fp, const char *file_name, char **html_file_name_ptr);

#endif /* RENDER_H */
