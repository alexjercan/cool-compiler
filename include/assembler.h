#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "semantic.h"

enum assembler_result {
    ASSEMBLER_OK = 0,
    ASSEMBLER_ERROR,
};

enum assembler_result assembler_run(const char *filename, semantic_mapping *mapping);

#endif // ASSEMBLER_H
