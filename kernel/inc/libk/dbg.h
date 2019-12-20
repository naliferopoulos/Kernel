#ifndef DBG_H
#define DBG_H

#include <arch/regs.h>
#include <libk/types.h>

typedef struct {
	uint32_t address;
	char * name;
} symbol_t;

#define EOF (-1)

#define ASSERT(b) ((b) ? (void)0 : kpanicAssert(__FILE__, __LINE__, #b))

#define dbg(x, ...) dbgf(x, ##__VA_ARGS__)

int dbgf(const char* __restrict, ...);
void kstrace(uint32_t dummy);
void kpanicAssert(char *file, int line, char *desc);
void kpanic(char* err, struct regs* r);
void abort();
void set_symbol_map(uint32_t addr);

#endif
