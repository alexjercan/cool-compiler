#include "util.h"
#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

// TODO: Add support for the following:
// - class_nameTable
// - class_objTable
//   - X_protObj
//     - X_dispTable
//       - X_methods
//     - attributes ...
//   - X_init

int main(int argc, char **argv) {
    int result = 0;
    char *buffer = NULL;
    program_node program;
    ds_dynamic_array tokens;
    struct argparse_parser *parser = NULL;
    struct semantic_mapping mapping = {0};

    ds_dynamic_array_init(&tokens, sizeof(struct token));

    parser = util_parse_arguments(argc, argv);
    if (parser == NULL) {
        DS_LOG_ERROR("Failed to parse arguments");
        return_defer(1);
    }

    char *filename = argparse_get_value(parser, ARG_INPUT);
    const char *basename = util_filepath_to_basename(filename);
    char *output = argparse_get_value(parser, ARG_OUTPUT);

    int length = util_read_file(filename, &buffer);
    if (length < 0) {
        DS_LOG_ERROR("Failed to read file: %s", filename);
        return_defer(1);
    }

    if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
        DS_LOG_ERROR("Failed to tokenize input");
        return_defer(1);
    }

    if (argparse_get_flag(parser, ARG_LEXER) == 1) {
        lexer_print_tokens(&tokens);
        return_defer(0);
    }

    if (parser_run(basename, &tokens, &program) != PARSER_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    if (argparse_get_flag(parser, ARG_SYNTAX) == 1) {
        parser_print_ast(&program);
        return_defer(0);
    }

    if (semantic_check(basename, &program, &mapping) != SEMANTIC_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    if (argparse_get_flag(parser, ARG_SEMANTIC) == 1) {
        parser_print_ast(&program);
        return_defer(0);
    }

    if (argparse_get_flag(parser, ARG_MAPPING) == 1) {
        semantic_print_mapping(&mapping);
        return_defer(0);
    }

    if (argparse_get_flag(parser, ARG_TACGEN) == 1) {
        codegen_tac_print(&program);
        return_defer(0);
    }

    if (assembler_run(output, &program, &mapping) != ASSEMBLER_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    if (argparse_get_flag(parser, ARG_ASSEMBLER) == 1) {
        return_defer(0);
    }

defer:
    for (unsigned int i = 0; i < tokens.count; i++) {
        struct token tok;
        ds_dynamic_array_get(&tokens, i, &tok);
        if (tok.literal)
            DS_FREE(tok.literal);
    }
    ds_dynamic_array_free(&tokens);
    if (buffer != NULL)
        DS_FREE(buffer);
    argparse_free(parser);
    return result;
}
