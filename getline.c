#include "./headers/unistd.h"

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream) {
  register char c;
  register char *cs = NULL;
  register int length = 0;
  while ((c = getc(stream)) != EOF) {
    cs = (char *)realloc(cs, ++length + 1);

    if ((*(cs + length - 1) = c) == '\n') {
      *(cs + length) = '\0';
      break;
    }
  }

  // return the allocated memory if lineptr is null
  if ((*lineptr) == NULL) *lineptr = cs;
  else {
    // check if enough memory is allocated 
    if ((length + 1) < *n) *lineptr = (char *)realloc(*lineptr, length + 1);

    memcpy(*lineptr, cs, length);
    free(cs);
  }

  return (ssize_t)(*n = strlen(*lineptr));
}