#ifndef UTIL_H
#define UTIL_H

void util_pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
                    unsigned int *col);

const char *filepath_to_basename(const char *path);

#endif // UTIL_H
