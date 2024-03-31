#include "util.h"
#include "ds.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
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
    ds_linked_list dirs_queue;

    ds_linked_list_init(&dirs_queue, sizeof(char *));
    ds_dynamic_array_init(filepaths, sizeof(char *));

    if (ds_linked_list_push_back(&dirs_queue, &dirpath) != 0) {
        DS_LOG_ERROR("Failed to push directory path to queue");
        return_defer(1);
    }

    while (ds_linked_list_empty(&dirs_queue) == 0) {
        char *dirpath;
        if (ds_linked_list_pop_front(&dirs_queue, &dirpath) != 0) {
            DS_LOG_ERROR("Failed to pop directory path from queue");
            return_defer(1);
        }

        DIR *dir = opendir(dirpath);
        if (dir == NULL) {
            DS_LOG_ERROR("Failed to open directory: %s", strerror(errno));
            return_defer(1);
        }

        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") == 0 ||
                    strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char *subdirpath;
                if (util_append_path(dirpath, entry->d_name, &subdirpath) != 0) {
                    DS_LOG_ERROR("Failed to append path to directory");
                    return_defer(1);
                }

                if (ds_linked_list_push_back(&dirs_queue, &subdirpath) != 0) {
                    DS_LOG_ERROR("Failed to push directory path to queue");
                    return_defer(1);
                }
            } else {
                char *filepath;
                if (util_append_path(dirpath, entry->d_name, &filepath) != 0) {
                    DS_LOG_ERROR("Failed to append path to file");
                    return_defer(1);
                }

                if (ds_dynamic_array_append(filepaths, &filepath) != 0) {
                    DS_LOG_ERROR("Failed to push file path to array");
                    return_defer(1);
                }
            }
        }
    }

defer:
    return result;
}

int util_list_dirs(const char *dirpath, ds_dynamic_array *dirs) {
    int result = 0;

    ds_dynamic_array_init(dirs, sizeof(char *));

    DIR *dir = opendir(dirpath);
    if (dir == NULL) {
        DS_LOG_ERROR("Failed to open directory: %s", strerror(errno));
        return_defer(1);
    }

    struct dirent *entry = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char *subdirpath = strdup(entry->d_name);

            if (ds_dynamic_array_append(dirs, &subdirpath) != 0) {
                DS_LOG_ERROR("Failed to push directory path to array");
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

    if (ds_string_builder_append(&sb, "/") != 0) {
        DS_LOG_ERROR("Failed to append slash to string builder");
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

int util_append_extension(const char *filename, const char *extension,
                          char **buffer) {
    int result = 0;

    ds_string_builder sb;
    ds_string_builder_init(&sb);

    if (ds_string_builder_append(&sb, filename) != 0) {
        DS_LOG_ERROR("Failed to append filename to string builder");
        return_defer(1);
    }

    if (ds_string_builder_append(&sb, ".") != 0) {
        DS_LOG_ERROR("Failed to append period to string builder");
        return_defer(1);
    }

    if (ds_string_builder_append(&sb, extension) != 0) {
        DS_LOG_ERROR("Failed to append extension to string builder");
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

int util_cwd(char **buffer) {
    int result = 0;
    char *cwd = getcwd(NULL, 0);

    if (cwd == NULL) {
        DS_LOG_ERROR("Failed to get current working directory: %s",
                     strerror(errno));
        return_defer(1);
    }

    *buffer = cwd;

    return_defer(0);

defer:
    return result;
}

int util_exec(const char *command, char *const argv[]) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1) {
        DS_LOG_ERROR("Failed to fork process: %s", strerror(errno));
        return 1;
    }

    if (pid == 0) {
        execvp(command, argv);
        DS_LOG_ERROR("Failed to execute command: %s", strerror(errno));
        return 1;
    }

    if (waitpid(pid, &status, 0) == -1) {
        DS_LOG_ERROR("Failed to wait for child process: %s", strerror(errno));
        return 1;
    }

    return status;
}
