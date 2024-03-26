#include "util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARGPARSE_IMPLEMENTATION
#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

// Add support for the following:
// - abort for dispatch on void
// - abort for case on void
// - abort for case on no match
// - exception handling
//
// Future plans:
// - add support for module based compile
// - add linker phase in compiler
// - implement allocator for memory management

#define FASM "fasm"
#define LD "ld"
#define DEFAULT_OUTPUT "main"
#define COMPILATION_HALTED()                                                   \
    do {                                                                       \
        fprintf(stderr, "Compilation halted\n");                               \
    } while (0)

enum status_code {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_STOP = 2,
};

typedef struct build_context {
        char *binary;
        ds_argparse_parser parser;
        ds_dynamic_array prelude_filepaths; // const char *
        ds_dynamic_array user_filepaths;    // const char *
        ds_dynamic_array asm_filepaths;     // const char *

        ds_dynamic_array user_programs; // program_node
        program_node program;
        semantic_mapping mapping;
} build_context;

static int build_context_prelude_init(build_context *context) {
    int result = 0;
    char *cool_home = NULL;
    char *cool_lib = NULL;
    ds_dynamic_array filepaths;
    ds_dynamic_array modules;

    ds_dynamic_array_init(&filepaths, sizeof(const char *));
    ds_dynamic_array_init(&modules, sizeof(const char *));

    cool_home = getenv("COOL_HOME");
    if (cool_home == NULL) {
        util_cwd(&cool_home);
    }

    if (util_append_path(cool_home, "lib", &cool_lib) != 0) {
        DS_LOG_ERROR("Failed to append path");
        return_defer(1);
    }

    ds_argparse_get_values(&context->parser, ARG_MODULE, &modules);

    if (util_append_default_modules(&modules) != 0) {
        DS_LOG_ERROR("Failed to append default modules");
        return_defer(1);
    }

    for (size_t i = 0; i < modules.count; i++) {
        char *module = NULL;
        ds_dynamic_array_get(&modules, i, (void **)&module);

        if (util_validate_module(module) != 0) {
            DS_LOG_ERROR("Module name %s is invalid. Valid options are: %s",
                         module, util_show_valid_modules());
            return_defer(1);
        }

        char *module_path = NULL;
        if (util_append_path(cool_lib, module, &module_path) != 0) {
            DS_LOG_ERROR("Failed to append path");
            return_defer(1);
        }

        if (util_list_filepaths(module_path, &filepaths) != 0) {
            DS_LOG_ERROR("Failed to list filepaths");
            return_defer(1);
        }

        for (size_t i = 0; i < filepaths.count; i++) {
            char *filepath = NULL;
            ds_dynamic_array_get(&filepaths, i, (void **)&filepath);

            ds_string_slice slice, ext;
            ds_string_slice_init(&slice, filepath, strlen(filepath));
            while (ds_string_slice_tokenize(&slice, '.', &ext) == 0) {
            }
            char *extension = NULL;
            ds_string_slice_to_owned(&ext, &extension);

            if (strcmp(extension, "cl") == 0) {
                if (ds_dynamic_array_append(&context->prelude_filepaths,
                                            &filepath) != 0) {
                    DS_LOG_ERROR("Failed to append filepath");
                    return_defer(1);
                }
            } else if (strcmp(extension, "asm") == 0) {
                if (ds_dynamic_array_append(&context->asm_filepaths,
                                            &filepath) != 0) {
                    DS_LOG_ERROR("Failed to append filepath");
                    return_defer(1);
                }
            }
        }
    }

defer:
    return result;
}

static int build_context_init(build_context *context,
                              ds_argparse_parser parser) {
    int result = 0;

    context->parser = parser;

    ds_dynamic_array_init(&context->prelude_filepaths, sizeof(const char *));
    ds_dynamic_array_init(&context->user_filepaths, sizeof(const char *));
    ds_dynamic_array_init(&context->asm_filepaths, sizeof(const char *));

    if (build_context_prelude_init(context) != 0) {
        DS_LOG_ERROR("Failed to initialize prelude");
        return_defer(1);
    }

    ds_argparse_get_values(&parser, ARG_INPUT, &context->user_filepaths);

    ds_dynamic_array_init(&context->user_programs, sizeof(program_node));
    ds_dynamic_array_init(&context->program.classes, sizeof(class_node));
    context->mapping = (struct semantic_mapping){0};

defer:
    return result;
}

static enum status_code parse_prelude(build_context *context) {
    int length;
    program_node program;
    char *buffer = NULL;
    ds_dynamic_array tokens; // struct token

    enum parser_result parser_status = PARSER_OK;

    enum status_code result = STATUS_OK;

    for (size_t i = 0; i < context->prelude_filepaths.count; i++) {
        const char *filepath = NULL;
        ds_dynamic_array_get(&context->prelude_filepaths, i,
                             (void **)&filepath);

        length = util_read_file(filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", filepath);
            return_defer(STATUS_ERROR);
        }

        // tokenize prelude
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(STATUS_ERROR);
        }

        // parse tokens
        if (parser_run(filepath, &tokens, &program) != PARSER_OK) {
            parser_status = PARSER_ERROR;
            continue;
        }

        for (unsigned int j = 0; j < program.classes.count; j++) {
            class_node *c = NULL;
            ds_dynamic_array_get_ref(&program.classes, j, (void **)&c);

            if (ds_dynamic_array_append(&context->program.classes, c) != 0) {
                DS_LOG_ERROR("Failed to append class");
                return_defer(1);
            }
        }
    }

    if (parser_status != PARSER_OK) {
        return_defer(STATUS_ERROR);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code parse_user(build_context *context) {
    int length;
    program_node program;
    char *buffer = NULL;
    ds_dynamic_array tokens; // struct token

    int lexer_stop = ds_argparse_get_flag(&context->parser, ARG_LEXER);
    int parser_stop = ds_argparse_get_flag(&context->parser, ARG_SYNTAX);

    enum parser_result parser_status = PARSER_OK;

    int result = STATUS_OK;

    for (size_t i = 0; i < context->user_filepaths.count; i++) {
        const char *filepath = NULL;
        ds_dynamic_array_get(&context->user_filepaths, i, (void **)&filepath);

        // read input file
        length = util_read_file(filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", filepath);
            return_defer(STATUS_ERROR);
        }

        // tokenize input
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(STATUS_ERROR);
        }

        if (lexer_stop == 1) {
            lexer_print_tokens(&tokens);
            continue;
        }

        // parse tokens
        program_node program;
        if (parser_run(filepath, &tokens, &program) != PARSER_OK) {
            parser_status = PARSER_ERROR;
            continue;
        }

        for (unsigned int j = 0; j < program.classes.count; j++) {
            class_node *c = NULL;
            ds_dynamic_array_get_ref(&program.classes, j, (void **)&c);

            if (ds_dynamic_array_append(&context->program.classes, c) != 0) {
                DS_LOG_ERROR("Failed to append class");
                return_defer(STATUS_ERROR);
            }
        }

        if (ds_dynamic_array_append(&context->user_programs, &program) != 0) {
            DS_LOG_ERROR("Failed to append program");
            return_defer(1);
        }
    }

    if (lexer_stop == 1) {
        return_defer(STATUS_STOP);
    }

    if (parser_status != PARSER_OK) {
        return_defer(STATUS_ERROR);
    }

    if (parser_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i,
                                     (void **)&program);
            parser_print_ast(program);
        }
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code gatekeeping(build_context *context) {
    int semantic_stop = ds_argparse_get_flag(&context->parser, ARG_SEMANTIC);
    int mapping_stop = ds_argparse_get_flag(&context->parser, ARG_MAPPING);

    int result = STATUS_OK;

    if (semantic_check(&context->program, &context->mapping) != SEMANTIC_OK) {
        return_defer(STATUS_ERROR);
    }

    if (semantic_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i,
                                     (void **)&program);
            parser_print_ast(program);
        }
        return_defer(STATUS_STOP);
    }

    if (mapping_stop == 1) {
        semantic_print_mapping(&context->mapping);
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code codegen(build_context *context) {
    int length;
    char *buffer = NULL;

    int tacgen_stop = ds_argparse_get_flag(&context->parser, ARG_TACGEN);
    int assembler_stop = ds_argparse_get_flag(&context->parser, ARG_ASSEMBLER);
    char *output = ds_argparse_get_value(&context->parser, ARG_OUTPUT);
    char *asm_path = NULL;

    int result = STATUS_OK;

    if (output == NULL) {
        output = DEFAULT_OUTPUT;
    }

    if (assembler_stop == 0 && util_append_extension(output, "asm", &asm_path) != 0) {
        DS_LOG_ERROR("Failed to append extension");
        return_defer(STATUS_ERROR);
    }

    if (tacgen_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i,
                                     (void **)&program);
            codegen_tac_print(&context->mapping, program);
        }
        return_defer(STATUS_STOP);
    }

    if (util_write_file(asm_path, "format ELF64\n", "w") != 0) {
        DS_LOG_ERROR("Failed to write file: %s", asm_path);
        return_defer(STATUS_ERROR);
    }

    for (size_t i = 0; i < context->asm_filepaths.count; i++) {
        const char *asm_filepath = NULL;
        ds_dynamic_array_get(&context->asm_filepaths, i,
                             (void **)&asm_filepath);

        // read asm prelude file
        length = util_read_file(asm_filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", asm_filepath);
            return_defer(STATUS_ERROR);
        }

        if (util_write_file(asm_path, buffer, "a") != 0) {
            DS_LOG_ERROR("Failed to write file: %s", asm_path);
            return_defer(STATUS_ERROR);
        }
    }

    // assembler
    if (assembler_run(asm_path, &context->mapping) != ASSEMBLER_OK) {
        return_defer(STATUS_ERROR);
    }

    if (assembler_stop == 1) {
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code fasm_run(build_context *context) {
    enum status_code result = STATUS_OK;
    char *command = NULL;

    ds_string_builder sb;
    ds_string_builder_init(&sb);

    char *output = ds_argparse_get_value(&context->parser, ARG_OUTPUT);
    char *asm_path = NULL;

    if (output == NULL) {
        output = DEFAULT_OUTPUT;
    }

    if (util_append_extension(output, "asm", &asm_path) != 0) {
        DS_LOG_ERROR("Failed to append extension");
        return_defer(STATUS_ERROR);
    }

    if (ds_string_builder_append(&sb, "%s %s", FASM, asm_path) != 0) {
        DS_LOG_ERROR("Failed to append flag to string builder");
        return_defer(STATUS_ERROR);
    }

    ds_string_builder_build(&sb, &command);

    DS_LOG_INFO("Executing command: %s", command);

    if (util_exec(FASM, (char *const[]){FASM, asm_path, NULL}) != 0) {
        DS_LOG_ERROR("Failed to execute fasm");
        return_defer(STATUS_ERROR);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code ld_run(build_context *context) {
    enum status_code result = STATUS_OK;

    char **ld_flags_array = NULL;
    char *command = NULL;

    ds_string_builder sb;
    ds_string_builder_init(&sb);

    ds_dynamic_array ld_flags;
    ds_dynamic_array modules;

    ds_dynamic_array_init(&ld_flags, sizeof(const char *));
    ds_dynamic_array_init(&modules, sizeof(const char *));

    ds_argparse_get_values(&context->parser, ARG_MODULE, &modules);

    char *output = ds_argparse_get_value(&context->parser, ARG_OUTPUT);
    char *obj_path = NULL;

    if (output == NULL) {
        output = DEFAULT_OUTPUT;
    }

    if (util_append_extension(output, "o", &obj_path) != 0) {
        DS_LOG_ERROR("Failed to append extension");
        return_defer(STATUS_ERROR);
    }

    if (util_get_ld_flags(modules, &ld_flags) != 0) {
        DS_LOG_ERROR("Failed to get ld flags");
        return_defer(STATUS_ERROR);
    }

    int needed = ld_flags.count + 5;
    ld_flags_array = malloc(sizeof(char *) * needed);
    if (ld_flags_array == NULL) {
        DS_LOG_ERROR("Failed to allocate memory for ld flags");
        return_defer(STATUS_ERROR);
    }

    ld_flags_array[0] = LD;
    ld_flags_array[1] = "-o";
    ld_flags_array[2] = output;
    ld_flags_array[3] = obj_path;
    if (ds_string_builder_append(&sb, "%s -o %s %s ", LD, output, obj_path) != 0) {
        DS_LOG_ERROR("Failed to append flag to string builder");
        return_defer(STATUS_ERROR);
    }

    for (size_t i = 0; i < ld_flags.count; i++) {
        char *flag = NULL;
        ds_dynamic_array_get(&ld_flags, i, &flag);

        if (ds_string_builder_append(&sb, "%s ", flag) != 0) {
            DS_LOG_ERROR("Failed to append flag to string builder");
            return_defer(STATUS_ERROR);
        }

        ld_flags_array[i + 4] = flag;
    }

    ld_flags_array[needed - 1] = NULL;

    ds_string_builder_build(&sb, &command);
    DS_LOG_INFO("Executing command: %s", command);

    if (util_exec(LD, ld_flags_array) != 0) {
        DS_LOG_ERROR("Failed to execute ld");
        return_defer(STATUS_ERROR);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

int main(int argc, char **argv) {
    int result = 0;
    build_context context;
    ds_argparse_parser parser;

    if (util_parse_arguments(&parser, argc, argv) != 0) {
        DS_LOG_ERROR("Failed to parse arguments");
        return_defer(1);
    }

    build_context_init(&context, parser);

    int prelude_result = parse_prelude(&context);
    int user_result = parse_user(&context);
    if (prelude_result == STATUS_STOP || user_result == STATUS_STOP) {
        return_defer(0);
    }
    if (prelude_result != STATUS_OK || user_result != STATUS_OK) {
        COMPILATION_HALTED();
        return_defer(1);
    }

    int gatekeeping_result = gatekeeping(&context);
    if (gatekeeping_result == STATUS_STOP) {
        return_defer(0);
    }
    if (gatekeeping_result != STATUS_OK) {
        COMPILATION_HALTED();
        return_defer(1);
    }

    int codegen_result = codegen(&context);
    if (codegen_result == STATUS_STOP) {
        return_defer(0);
    }
    if (codegen_result != STATUS_OK) {
        COMPILATION_HALTED();
        return_defer(1);
    }

    int fasm_result = fasm_run(&context);
    if (fasm_result == STATUS_STOP) {
        return_defer(0);
    }
    if (fasm_result != STATUS_OK) {
        COMPILATION_HALTED();
        return_defer(1);
    }

    int ld_result = ld_run(&context);
    if (ld_result == STATUS_STOP) {
        return_defer(0);
    }
    if (ld_result != STATUS_OK) {
        COMPILATION_HALTED();
        return_defer(1);
    }

    return_defer(0);

defer:
    // TODO: memory
    return result;
}
