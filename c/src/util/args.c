#include "util.h"
#include "argparse.h"
#include <stdio.h>

struct argparse_parser *util_parse_arguments(int argc, char **argv) {
    struct argparse_parser *parser = NULL;

    parser = argparse_new(PROGRAM_NAME, PROGRAM_DESCRIPTION, PROGRAM_VERSION);

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'l',
                                           .long_name = ARG_LEXER,
                                           .description = "Lex the input file",
                                           .type = ARGUMENT_TYPE_FLAG,
                                           .required = 0}));

    argparse_add_argument(parser, ((struct argparse_options){
                                      .short_name = 's',
                                      .long_name = ARG_SYNTAX,
                                      .description = "Parse the input file",
                                      .type = ARGUMENT_TYPE_FLAG,
                                      .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'S',
                                           .long_name = ARG_SEMANTIC,
                                           .description = "Semantic check the input file",
                                           .type = ARGUMENT_TYPE_FLAG,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'i',
                                           .long_name = ARG_INPUT,
                                           .description = "Input file",
                                           .type = ARGUMENT_TYPE_POSITIONAL,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'o',
                                           .long_name = ARG_OUTPUT,
                                           .description = "Output file",
                                           .type = ARGUMENT_TYPE_VALUE,
                                           .required = 0}));

    if (argparse_parse(parser, argc, argv) != 0) {
        return NULL;
    }

    return parser;
}
