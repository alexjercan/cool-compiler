#include "util.h"

void util_pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
                      unsigned int *col) {
    *line = 1;
    *col = 1;
    for (unsigned int i = 0; i < pos; i++) {
        if (buffer[i] == '\n') {
            *line += 1;
            *col = 1;
        } else {
            *col += 1;
        }
    }
}
