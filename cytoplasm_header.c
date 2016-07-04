#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cytoplasm_header.h"

#define CYTO_HEADER_BORDER "---"

char
*read_line(FILE *fp)
{
    int ch; /* Character/byte */

    /* Get the line length */
    char *line;
    int line_length = 0;
    while ((ch = fgetc(fp)) != EOF && ch != '\n') {
        line_length++;
    }
    fseek(fp, -1 * line_length - 1, SEEK_CUR); /* Rewind */

    /* Read the line */
    line = malloc(line_length + 1);
    memset(line, 0, line_length + 1);
    int i;
    for (i = 0; i < line_length; i++) {
        ch = fgetc(fp);
        line[i] = ch;
    }
    fgetc(fp); /* Throw away the newline/EOF */

    return line;
}

void
cytoplasm_header_read(FILE *fp, ctache_data_t *data)
{
    char *line;
    size_t line_len;
    int ch;
    char *key;
    char *value;
    int i;
    int index;

    key = NULL;
    value = NULL;
    line = read_line(fp);
    line_len = strlen(line);
    if (strcmp(line, CYTO_HEADER_BORDER) == 0) {
        line = read_line(fp);
        line_len = strlen(line);
        while (strcmp(line, CYTO_HEADER_BORDER) != 0 && !feof(fp)) {
            key = malloc(line_len + 1);
            value = malloc(line_len + 1);
            for (i = 0; i < line_len; i++) {
                ch = line[i];
                if (ch == ':') {
                    index = i + 1;
                    break;
                }
                key[i] = ch;
            }
            for (i = index; i < line_len; i++) {
                ch = line[i];
                value[i - index] = ch;
            }

            ctache_data_hash_table_set(data, key, value);

            value = NULL;
            free(key);
            key = NULL;
            free(line);

            line = read_line(fp);
            line_len = strlen(line);
        }
    } else {
        /* Rewind the file since there is no header */
        fseek(fp, -1 * line_len - 1, SEEK_CUR);
    }
    if (key != NULL) {
        free(key);
    }
    free(line);
}
