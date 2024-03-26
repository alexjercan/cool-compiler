#include "util.h"
#include <stdio.h>
#include "ds.h"

int util_read_file(const char *filename, char **buffer) {
    int result = 0;
    FILE *file = NULL;
    ds_string_builder sb;
    ds_string_builder_init(&sb);

    if (filename != NULL) {
        file = fopen(filename, "r");
        if (file == NULL) {
            DS_LOG_ERROR("Failed to open file: %s", filename);
            return_defer(-1);
        }
    } else {
        file = stdin;
    }

    char line[LINE_MAX] = {0};
    while (fgets(line, sizeof(line), file) != NULL) {
        unsigned int len = 0;
        for (len = 0; len < LINE_MAX; len++) {
            if (line[len] == '\n' || line[len] == EOF) {
                break;
            }
        }

        if (len == LINE_MAX) {
            len = strlen(line);
        } else {
            len += 1;
            line[len] = '\0';
        }

        if (len == LINE_MAX) {
            DS_LOG_ERROR("Line too long");
            return_defer(-1);
        }

        if (ds_string_builder_appendn(&sb, line, len) != 0) {
            DS_LOG_ERROR("Failed to append line to string builder");
            return_defer(-1);
        }

        memset(line, 0, sizeof(line));
    }

    if (ds_string_builder_build(&sb, buffer) != 0) {
        DS_LOG_ERROR("Failed to build string from string builder");
        return_defer(-1);
    }

    result = sb.items.count;

defer:
    if (filename != NULL && file != NULL)
        fclose(file);
    ds_string_builder_free(&sb);
    return result;
}

int util_write_file(const char *filename, char *buffer, const char *mode) {
    int result = 0;
    FILE *file = NULL;

    if (filename != NULL) {
        file = fopen(filename, mode);
        if (file == NULL) {
            DS_LOG_ERROR("Failed to open file: %s", filename);
            return_defer(-1);
        }
    } else {
        file = stdout;
    }

    if (fputs(buffer, file) == EOF) {
        DS_LOG_ERROR("Failed to write to file");
        return_defer(-1);
    }

    result = 0;

defer:
    if (filename != NULL && file != NULL)
        fclose(file);
    return result;
}
