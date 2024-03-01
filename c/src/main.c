#include "util.h"
#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "ds.h"
#include "io.h"
#include "lexer.h"
#include "parser.h"
#include "print_ast.h"
#include "semantic.h"

int main(int argc, char **argv) {
    int result = 0;
    char *buffer = NULL;
    ds_dynamic_array tokens;
    ds_dynamic_array_init(&tokens, sizeof(struct token));
    program_node program;
    struct argparse_parser *parser = NULL;

    parser = argparse_new("coolc", "A cool compiler", "0.1.0");

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'l',
                                           .long_name = "lex",
                                           .description = "Lex the input file",
                                           .type = ARGUMENT_TYPE_FLAG,
                                           .required = 0}));

    argparse_add_argument(parser, ((struct argparse_options){
                                      .short_name = 's',
                                      .long_name = "syn",
                                      .description = "Parse the input file",
                                      .type = ARGUMENT_TYPE_FLAG,
                                      .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'S',
                                           .long_name = "sem",
                                           .description = "Semantic check the input file",
                                           .type = ARGUMENT_TYPE_FLAG,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'i',
                                           .long_name = "input",
                                           .description = "Input file",
                                           .type = ARGUMENT_TYPE_POSITIONAL,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'o',
                                           .long_name = "output",
                                           .description = "Output file",
                                           .type = ARGUMENT_TYPE_VALUE,
                                           .required = 0}));

    if (argparse_parse(parser, argc, argv) != 0) {
        DS_LOG_ERROR("Failed to parse arguments");
        return_defer(1);
    }

    // Read the input file
    char *filename = argparse_get_value(parser, "input");
    int length = read_file(filename, &buffer);
    if (length < 0) {
        DS_LOG_ERROR("Failed to read file: %s", filename);
        return_defer(1);
    }

    // Lex the input file
    lexer_tokenize(buffer, length, &tokens);

    // If the lex flag is set, print the tokens
    unsigned int lex = argparse_get_flag(parser, "lex");
    if (lex == 1) {
        for (unsigned int i = 0; i < tokens.count; i++) {
            struct token tok;
            ds_dynamic_array_get(&tokens, i, &tok);

            printf("%s", token_type_to_string(tok.type));
            if (tok.type == ILLEGAL) {
                printf("(%s", error_type_to_string(tok.error));
                if (tok.literal != NULL) {
                    printf(" %s", tok.literal);
                }
                printf(")");
            } else if (tok.literal != NULL) {
                printf("(%s)", tok.literal);
            }
            printf("\n");
        }

        return_defer(0);
    }

    // Parse the tokens
    if (parser_run(filepath_to_basename(filename), &tokens, &program) != 0) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    // If the syn flag is set, parse the input file
    unsigned int syn = argparse_get_flag(parser, "syn");
    if (syn == 1) {
        print_ast(&program);
        return_defer(0);
    }


    // Semantic check the input file
    unsigned int sem = argparse_get_flag(parser, "sem");
    program_context context = { .filename = filepath_to_basename(filename) };
    if (semantic_check(&program, &context) != 0) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    if (sem == 1) {
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
