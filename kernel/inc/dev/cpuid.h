#ifndef CPUID_H
#define CPUID_H

#include <libk/types.h>

#define cpuid(in, a, b, c, d) __asm__ __volatile__("cpuid": "=a"(a), "=b"(b), "=c"(c), "=d"(d): "a"(in));

char* cpuid_name();

#endif
