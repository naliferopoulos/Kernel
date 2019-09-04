#ifndef HEAP_H
#define HEAP_H

#include <libk/types.h>

#define HEAP_POS 0xD0000000

void* kmalloc(size_t size);
void kfree(void* ptr);

#endif