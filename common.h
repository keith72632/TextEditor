#include <ctype.h>
#include <stdio.h>

#ifndef COMMON_H
#define COMMON_H

#define MARY_VERSION "0.0.1"
#define ABUF_INIT {NULL, 0}

struct abuf {
  char *b;
  int len;
};


void charInfo(char c);
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif