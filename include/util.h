#ifndef UTIL_H
#define UTIL_H

#include "ds.h"

#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

#define PROGRAM_NAME "coolc"
#define PROGRAM_DESCRIPTION "The Cool Programming Language Compiler"
#define PROGRAM_VERSION "0.1.0"

#define ARG_INPUT "input"
#define ARG_OUTPUT "output"

#define ARG_LEXER "lex"
#define ARG_SYNTAX "syn"
#define ARG_SEMANTIC "sem"
#define ARG_MAPPING "map"
#define ARG_TACGEN "tac"
#define ARG_ASSEMBLER "asm"

int util_parse_arguments(ds_argparse_parser *parser, int argc, char **argv);
void util_pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
                    unsigned int *col);

int util_read_file(const char *filename, char **buffer);
int util_write_file(const char *filename, char *buffer);
int util_list_filepaths(const char *dirpath, ds_dynamic_array *filepaths);
int util_append_path(char *path, const char *filename, char **buffer);
int util_append_extension(const char *filename, const char *extension,
                          char **buffer);
int util_cwd(char **buffer);
int util_exec(const char *command, char *const argv[]);

#endif // UTIL_H
