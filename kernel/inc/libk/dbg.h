#ifndef DBG_H
#define DBG_H

#include <arch/regs.h>

#define EOF (-1)

#define ASSERT(b) ((b) ? (void)0 : kpanicAssert(__FILE__, __LINE__, #b))

#define dbg(x, ...) dbgf(x, ##__VA_ARGS__)

int dbgf(const char* __restrict, ...);
void kstrace(int depth);
void kpanicAssert(char *file, int line, char *desc);
void kpanic(char* err, struct regs* r);
void abort();

#endif