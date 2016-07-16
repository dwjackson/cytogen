#include "layout.h"
#include "processing.h"
#include "files.h"
#include "string_util.h"
#include <pthread.h>
#include <ctache/ctache.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/mman.h>

#define LAYOUT "layout"

char
*determine_out_file_name(const char *in_file_name,
                         const char *site_dir,
                         pthread_mutex_t *basename_mtx)
{
    char *in_file_name_dup;
    char *out_file_name;
    size_t out_file_name_len;

    in_file_name_dup = strdup(in_file_name);
    pthread_mutex_lock(basename_mtx);
    char *in_file_base_name = basename(in_file_name_dup);
    pthread_mutex_unlock(basename_mtx);

    out_file_name_len = strlen(site_dir) + 1 + strlen(in_file_base_name);
    out_file_name = malloc(out_file_name_len + 1);
    strcpy(out_file_name, site_dir);
    strcat(out_file_name, "/");
    strcat(out_file_name, in_file_base_name);
    if (out_file_name == NULL) {
        fprintf(stderr, "ERROR: Could not malloc() for out_file_name\n");
        return;
    }

    free(in_file_name_dup);

    return out_file_name;
}

void
process_file(const char *in_file_name, struct process_file_args *args)
{
    char *in_file_extension;
    char *out_file_name;
    const char *site_dir = args->site_dir;

    in_file_extension = file_extension(in_file_name);
    out_file_name = determine_out_file_name(in_file_name,
                                            site_dir,
                                            args->basename_mutex);
    
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
}

void
*process_files(void *args_ptr)
{
    struct process_file_args *args = (struct process_file_args *)args_ptr;
    int i;
    char *in_file_name;
    for (i = args->start_index; i < args->end_index; i++) {
        in_file_name = (args->file_names)[i];
        process_file(in_file_name, args);
    }

    return NULL;
}
