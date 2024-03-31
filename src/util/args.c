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

