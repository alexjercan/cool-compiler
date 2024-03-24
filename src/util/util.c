#include "util.h"
#include "ds.h"
#include <dirent.h>
#include <errno.h>

void util_pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
                      unsigned int *col) {
    *line = 1;
    *col = 1;
    for (unsigned int i = 0; i < pos; i++) {
        if (buffer[i] == '\n') {
            *line += 1;
            *col = 1;
        } else {
            *col += 1;
        }
    }
}

int util_list_filepaths(const char *dirpath, ds_dynamic_array *filepaths) {
    int result = 0;
    struct dirent *entry;
    DIR *dir = opendir(dirpath);

    ds_dynamic_array_init(filepaths, sizeof(char *));

    if (dir == NULL) {
        DS_LOG_ERROR("Failed to open directory: %s", strerror(errno));
        return_defer(1);
    }

    // TODO: search for nested directories
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int needed = snprintf(NULL, 0, "%s/%s", dirpath, entry->d_name);
            char *filepath = malloc(needed + 1);
            if (filepath == NULL) {
                DS_LOG_ERROR("Failed to allocate memory for filepath");
                return_defer(1);
            }

            snprintf(filepath, needed + 1, "%s/%s", dirpath, entry->d_name);

            if (ds_dynamic_array_append(filepaths, &filepath) != 0) {
                DS_LOG_ERROR("Failed to append filepath to array");
                return_defer(1);
            }
        }
    }

defer:
    return result;
}

int util_append_path(char *path, const char *filename, char **buffer) {
    int result = 0;
    ds_string_builder sb;
    ds_string_builder_init(&sb);

    if (ds_string_builder_append(&sb, path) != 0) {
        DS_LOG_ERROR("Failed to append path to string builder");
        return_defer(1);
    }

    if (ds_string_builder_append(&sb, filename) != 0) {
        DS_LOG_ERROR("Failed to append filename to string builder");
        return_defer(1);
    }

    if (ds_string_builder_build(&sb, buffer) != 0) {
        DS_LOG_ERROR("Failed to build string from string builder");
        return_defer(1);
    }

    return_defer(0);

defer:
    return result;
}
