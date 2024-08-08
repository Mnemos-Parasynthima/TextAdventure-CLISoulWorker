#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN64
#define ssize_t int64_t
#else
#define ssize_t long
#endif

// typedef __int64   int64_t;
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

#endif // !_UNISTD_H