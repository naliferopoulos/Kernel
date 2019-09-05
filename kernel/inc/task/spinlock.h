#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <libk/types.h>

typedef struct {
	uint16_t lock;
} spinlock_t;

extern void acquire_spinlock(spinlock_t* spinlock);
extern void release_spinlock(spinlock_t* spinlock);

#endif