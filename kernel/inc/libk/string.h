#ifndef STRING_H
#define STRING_H

#include <libk/stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);
int strcmp(char *str1, char *str2);

#endif
