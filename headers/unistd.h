#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN64
typedef __int64   int64_t;
#define ssize_t __int64
#else
#define ssize_t long
#endif

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);

#endif