#ifndef ASCII_UTILS_H
#define ASCII_UTILS_H

#include "main.h"

char petToAsc(char c);
bool ascToPet(char *c, Stream *stream);
char ascToPetcii(char c);
bool handleAsciiIAC(char *c, Stream *stream);
void setCharArray(char **target, const char *src);
void freeCharArray(char **arr);
int modifierCompare(const char *match1, const char *match2);
char lc(char c);

#endif