#ifndef STDLIB_H
#define STDLIB_H

int atoi(const char *str);
int strtoi(const char *str, char **endp, int base);

void itoa(long val, char *buf, int base);
void utoa(unsigned long val, char *buf, int base);

void* kmemset(void* b, int c, int len);

#endif
