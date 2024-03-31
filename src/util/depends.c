#include "util.h"
#include "ds.h"

enum dependency_kind {
    DEPENDENCY,
    INCOMPATIBILITY,
};

typedef struct dependency {
    const char *module;
    ds_dynamic_array dependencies; // const char *
} dependency;

typedef struct incompatibility {
    const char *module1;
    const char *module2;
} incompatibility;

typedef struct instruction {
    enum dependency_kind kind;
    union {
        dependency dependency;
        incompatibility incompatibility;
    };
} instruction;

static int parse_dependency(const char *line, dependency *dep) {
    int result = 0;
    char *module = NULL;
    char *buffer = NULL;
    ds_string_slice line_slice;
    ds_string_slice token;

    ds_dynamic_array_init(&dep->dependencies, sizeof(const char *));
    ds_string_slice_init(&line_slice, (char *)line, strlen(line));

    ds_string_slice_tokenize(&line_slice, ':', &token);
    ds_string_slice_trim(&token, ' ');
    ds_string_slice_to_owned(&token, &module);
    if (module == NULL) {
        return_defer(1);
    }

    dep->module = module;

    while (ds_string_slice_tokenize(&line_slice, '|', &token) == 0) {
        ds_string_slice_trim(&token, ' ');
        ds_string_slice_to_owned(&token, &buffer);
        if (buffer == NULL) {
            return_defer(1);
        }

        if (ds_dynamic_array_append(&dep->dependencies, &buffer) != 0) {
            return_defer(1);
        }
    }

defer:
    return result;
}

static int parse_incompatibility(const char *line, incompatibility *inc) {
    int result = 0;
    char *module1 = NULL;
    char *module2 = NULL;
    ds_string_slice line_slice;
    ds_string_slice token;

    ds_string_slice_init(&line_slice, (char *)line, strlen(line));

    ds_string_slice_tokenize(&line_slice, '^', &token);
    ds_string_slice_trim(&token, ' ');
    ds_string_slice_to_owned(&token, &module1);
    if (module1 == NULL) {
        return_defer(1);
    }

    inc->module1 = module1;

    ds_string_slice_trim(&line_slice, ' ');
    ds_string_slice_to_owned(&line_slice, &module2);
    if (module2 == NULL) {
        return_defer(1);
    }

    inc->module2 = module2;

defer:
    return result;
}

static int string_contains(const char *str, char c) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == c) {
            return 1;
        }
    }

    return 0;
}

static int parse_instruction(const char *line, instruction *instr) {
    int result = 0;
    char *buffer = NULL;
    ds_string_slice line_slice;
    ds_string_slice token;

    ds_string_slice_init(&line_slice, (char *)line, strlen(line));

    if (string_contains(line, ':')) {
        instr->kind = DEPENDENCY;
        if (parse_dependency(line, &instr->dependency) != 0) {
            return_defer(1);
        }
    } else if (string_contains(line, '^')) {
        instr->kind = INCOMPATIBILITY;
        if (parse_incompatibility(line, &instr->incompatibility) != 0) {
            return_defer(1);
        }
    }

defer:
    return result;
}

static int parse_dependencies(const char *buffer, ds_dynamic_array *instructions) {
    int result = 0;
    ds_string_slice buffer_slice;
    char *line = NULL;
    ds_string_slice line_slice;
    ds_string_slice token;
    instruction instr;

    ds_dynamic_array_init(instructions, sizeof(instruction));
    ds_string_slice_init(&buffer_slice, (char *)buffer, strlen(buffer));

    while (ds_string_slice_tokenize(&buffer_slice, '\n', &line_slice) == 0) {
        ds_string_slice_trim(&line_slice, ' ');
        ds_string_slice_to_owned(&line_slice, &line);
        if (line == NULL) {
            return_defer(1);
        }

        if (parse_instruction(line, &instr) != 0) {
            return_defer(1);
        }

        if (ds_dynamic_array_append(instructions, &instr) != 0) {
            return_defer(1);
        }
    }

defer:
    return result;
}

static int array_contains(ds_dynamic_array *array, char *item) {
    for (size_t i = 0; i < array->count; i++) {
        char *array_item = NULL;
        ds_dynamic_array_get(array, i, &array_item);

        if (strcmp(array_item, item) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_incompatible(ds_dynamic_array instructions, ds_dynamic_array *modules, char *item) {
    for (size_t i = 0; i < instructions.count; i++) {
        instruction instr;
        ds_dynamic_array_get(&instructions, i, &instr);

        if (instr.kind == INCOMPATIBILITY) {
            for (size_t j = 0; j < modules->count; j++) {
                char *module = NULL;
                ds_dynamic_array_get(modules, j, &module);

                if (strcmp(module, instr.incompatibility.module1) == 0 && strcmp(item, instr.incompatibility.module2) == 0) {
                    return 1;
                }

                if (strcmp(module, instr.incompatibility.module2) == 0 && strcmp(item, instr.incompatibility.module1) == 0) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

int util_resolve_modules(char *buffer, char *cool_home,
                                ds_dynamic_array *modules /* char * */) {
    int result = 0;
    ds_dynamic_array instructions;

    if (parse_dependencies(buffer, &instructions) != 0) {
        return_defer(1);
    }

    for (size_t i = 0; i < modules->count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(modules, i, &module);

        if (is_incompatible(instructions, modules, module)) {
            DS_LOG_ERROR("Initial incompatibility found for %s", module);
            return_defer(1);
        }
    }

    for (size_t i = 0; i < modules->count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(modules, i, &module);

        for (size_t j = 0; j < instructions.count; j++) {
            instruction instr;
            ds_dynamic_array_get(&instructions, j, &instr);

            if (instr.kind == DEPENDENCY && strcmp(module, instr.dependency.module) == 0) {
                int found = 0;
                for (size_t k = 0; k < instr.dependency.dependencies.count; k++) {
                    char *dep = NULL;
                    ds_dynamic_array_get(&instr.dependency.dependencies, k, &dep);

                    if (is_incompatible(instructions, modules, dep)) {
                        continue;
                    }

                    found = 1;

                    if (array_contains(modules, dep)) {
                        break;
                    }

                    if (ds_dynamic_array_append(modules, &dep) != 0) {
                        return_defer(1);
                    }
                }

                if (!found) {
                    DS_LOG_ERROR("No valid dependency found for %s", module);
                    return_defer(1);
                }
            }
        }
    }

    ds_dynamic_array modules_unique;
    ds_dynamic_array_init(&modules_unique, sizeof(char *));

    for (size_t i = 0; i < modules->count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(modules, i, &module);

        if (array_contains(&modules_unique, module)) {
            continue;
        }

        if (ds_dynamic_array_append(&modules_unique, &module) != 0) {
            return_defer(1);
        }
    }

    modules->count = 0;

    for (size_t i = 0; i < modules_unique.count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(&modules_unique, i, &module);

        if (ds_dynamic_array_append(modules, &module) != 0) {
            return_defer(1);
        }
    }

defer:
    return result;
}

#define util_show_invalid_module_error(module)                                 \
    DS_LOG_ERROR(                                                              \
        "Module name %s is not a module in the cool standard library", module)

int util_validate_module(char *cool_lib, const char *module) {
    ds_dynamic_array dirs;

    if(util_list_dirs(cool_lib, &dirs) != 0) {
        return 1;
    }

    for (size_t i = 0; i < dirs.count; i++) {
        char *filepath = NULL;
        ds_dynamic_array_get(&dirs, i, &filepath);

        if (strcmp(filepath, module) == 0) {
            return 0;
        }
    }

    util_show_invalid_module_error(module);
    return 1;
}

static int util_replace(const char *str, const char *old, const char *new, char **buffer) {
    int result = 0;
    size_t old_len = strlen(old);
    size_t new_len = strlen(new);
    size_t str_len = strlen(str);
    size_t buffer_len = 0;
    size_t buffer_index = 0;
    size_t i = 0;

    for (i = 0; i < str_len; i++) {
        if (strncmp(str + i, old, old_len) == 0) {
            buffer_len += new_len;
            i += old_len - 1;
        } else {
            buffer_len++;
        }
    }

    *buffer = (char *)malloc(buffer_len + 1);
    if (*buffer == NULL) {
        return_defer(1);
    }

    for (i = 0; i < str_len; i++) {
        if (strncmp(str + i, old, old_len) == 0) {
            memcpy(*buffer + buffer_index, new, new_len);
            buffer_index += new_len;
            i += old_len - 1;
        } else {
            (*buffer)[buffer_index] = str[i];
            buffer_index++;
        }
    }

    (*buffer)[buffer_len] = '\0';

defer:
    return result;
}

int util_get_ld_flags(char *cool_home, ds_dynamic_array modules, ds_dynamic_array *ld_flags) {
    int result = 0;
    char *cool_lib = NULL;
    char *module_path = NULL;
    char *flags_path = NULL;
    char *flags_buffer = NULL;
    ds_string_slice flags_slice;
    ds_string_slice token;

    if (util_append_path(cool_home, "lib", &cool_lib) != 0) {
        return_defer(1);
    }

    for (size_t i = 0; i < modules.count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(&modules, i, &module);

        if (util_validate_module(cool_lib, module) != 0) {
            return_defer(1);
        }

        if (util_append_path(cool_lib, module, &module_path) != 0) {
            return_defer(1);
        }

        if (util_append_path(module_path, "flags.txt", &flags_path) != 0) {
            return_defer(1);
        }

        if (util_read_file(flags_path, &flags_buffer) < 0) {
            return_defer(1);
        }

        ds_string_slice_init(&flags_slice, flags_buffer, strlen(flags_buffer));

        while (ds_string_slice_tokenize(&flags_slice, '\n', &token) == 0) {
            char *flag = NULL;
            ds_string_slice_to_owned(&token, &flag);

            if (flag == NULL) {
                return_defer(1);
            }

            if (strstr(flag, "COOL_HOME") != NULL) {
                char *new_flag = NULL;
                if (util_replace(flag, "COOL_HOME", cool_home, &new_flag) != 0) {
                    return_defer(1);
                }
                flag = new_flag;
            }

            if (ds_dynamic_array_append(ld_flags, &flag) != 0) {
                return_defer(1);
            }
        }
    }

defer:
    return result;
}
