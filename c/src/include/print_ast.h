#ifndef PRINT_AST_H
#define PRINT_AST_H

#include "parser.h"

#ifndef INDENT_SIZE
#define INDENT_SIZE 2
#endif

void print_ast(program_node *program);

#endif // PRINT_AST_H
