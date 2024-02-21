#ifndef IO_H
#define IO_H

#ifndef LINE_MAX
#define LINE_MAX 4096
#endif

int read_file(const char *filename, char **buffer);

#endif // IO_H
