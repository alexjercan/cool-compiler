#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

enum semantic_result {
    SEMANTIC_OK = 0,
    SEMANTIC_ERROR,
};

enum semantic_result semantic_check(const char *filename,
                                    program_node *program);

#endif // SEMANTIC_H
