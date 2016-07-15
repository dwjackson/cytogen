#ifndef FILES_H
#define FILES_H

void
get_file_list(const char *dir_name,
              char ***file_names_ptr,
              int *num_files_ptr, 
              char ***directories_ptr,
              int *num_directories_ptr);

char
*file_extension(const char *file_name);

#endif /* FILES_H */
