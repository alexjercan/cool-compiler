#include "ds.h"
#include "util.h"

int util_parse_arguments(ds_argparse_parser *parser, int argc, char **argv) {
    ds_argparse_parser_init(parser, PROGRAM_NAME, PROGRAM_DESCRIPTION,
                            PROGRAM_VERSION);

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'i',
                                       .long_name = ARG_INPUT,
                                       .description = "Input file",
                                       .type = ARGUMENT_TYPE_POSITIONAL_REST,
                                       .required = 1}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'o',
                                       .long_name = ARG_OUTPUT,
                                       .description = "Output file",
                                       .type = ARGUMENT_TYPE_VALUE,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'l',
                                       .long_name = ARG_LEXER,
                                       .description = "Lex the input file",
                                       .type = ARGUMENT_TYPE_FLAG,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 's',
                                       .long_name = ARG_SYNTAX,
                                       .description = "Parse the input file",
                                       .type = ARGUMENT_TYPE_FLAG,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser,
        ((ds_argparse_options){.short_name = 'S',
                               .long_name = ARG_SEMANTIC,
                               .description = "Semantic check the input file",
                               .type = ARGUMENT_TYPE_FLAG,
                               .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'm',
                                       .long_name = ARG_MAPPING,
                                       .description = "Generate mapping",
                                       .type = ARGUMENT_TYPE_FLAG,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 't',
                                       .long_name = ARG_TACGEN,
                                       .description = "Generate TAC",
                                       .type = ARGUMENT_TYPE_FLAG,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'a',
                                       .long_name = ARG_ASSEMBLER,
                                       .description = "Run the assembler",
                                       .type = ARGUMENT_TYPE_FLAG,
                                       .required = 0}));

    ds_argparse_add_argument(
        parser, ((ds_argparse_options){.short_name = 'm',
                                       .long_name = ARG_MODULE,
                                       .description = "Module name",
                                       .type = ARGUMENT_TYPE_VALUE_ARRAY,
                                       .required = 0}));

    return ds_argparse_parse(parser, argc, argv);
}

int util_validate_module(const char *module) {
    if (strcmp(module, MODULE_PRELUDE) != 0 && strcmp(module, MODULE_DS) != 0 &&
        strcmp(module, MODULE_RAYLIB) != 0) {
        DS_LOG_ERROR("Invalid module: %s", module);
        return 1;
    }

    return 0;
}

int util_append_default_modules(ds_dynamic_array *modules) {
    char *prelude = MODULE_PRELUDE;
    char *ds = MODULE_DS;
    int found_pre = 0;
    int found_ds = 0;

    for (size_t i = 0; i < modules->count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(modules, i, &module);

        if (strcmp(module, MODULE_PRELUDE) == 0) {
            found_pre = 1;
        } else if (strcmp(module, MODULE_DS) == 0) {
            found_ds = 1;
        }
    }

    if (!found_pre) {
        ds_dynamic_array_append(modules, &prelude);
    }

    if (!found_ds) {
        ds_dynamic_array_append(modules, &ds);
    }

    return 0;
}

char *util_show_valid_modules(void) {
    char *result = NULL;
    int needed = snprintf(NULL, 0, "%s, %s, %s", MODULE_PRELUDE, MODULE_DS,
                          MODULE_RAYLIB);

    char *buffer = malloc(needed + 1);
    if (buffer == NULL) {
        DS_LOG_ERROR("Failed to allocate memory for module list");
        return_defer(NULL);
    }

    snprintf(buffer, needed + 1, "%s, %s, %s", MODULE_PRELUDE, MODULE_DS,
             MODULE_RAYLIB);
    return_defer(buffer);

defer:
    return result;
}

int util_get_ld_flags(ds_dynamic_array modules, ds_dynamic_array *ld_flags) {
    ds_dynamic_array_init(ld_flags, sizeof(char *));

    int dynamic = 0;
    int lraylib = 0;
    int lm = 0;
    int lc = 0;

    for (size_t i = 0; i < modules.count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(&modules, i, &module);

        if (strcmp(module, MODULE_PRELUDE) == 0) {
            continue;
        } else if (strcmp(module, MODULE_DS) == 0) {
            continue;
        } else if (strcmp(module, MODULE_RAYLIB) == 0) {
            dynamic = 1;
            lraylib = 1;
            lm = 1;
            lc = 1;
        }
    }

    if (dynamic) {
        char *dynamic_linker = "-dynamic-linker";
        ds_dynamic_array_append(ld_flags, &dynamic_linker);
        char *linker = "/lib64/ld-linux-x86-64.so.2";
        ds_dynamic_array_append(ld_flags, &linker);
    }
    if (lraylib) {
        char *raylib = "-lraylib";
        ds_dynamic_array_append(ld_flags, &raylib);
        char *raylibpath = "-L./external/raylib/src";
        ds_dynamic_array_append(ld_flags, &raylibpath);
    }
    if (lm) {
        char *lm = "-lm";
        ds_dynamic_array_append(ld_flags, &lm);
    }
    if (lc) {
        char *lc = "-lc";
        ds_dynamic_array_append(ld_flags, &lc);
    }

    // TODO: defer
    return 0;
}
