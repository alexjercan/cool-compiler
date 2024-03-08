#ifndef UTIL_H
#define UTIL_H

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

struct argparse_parser *util_parse_arguments(int argc, char **argv);
void util_pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
                    unsigned int *col);

int util_read_file(const char *filename, char **buffer);
const char *util_filepath_to_basename(const char *path);

#endif // UTIL_H
