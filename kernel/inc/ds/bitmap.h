#ifndef BITMAP_H
#define BITMAP_H

#include <libk/types.h>
#include <libk/stdbool.h>

typedef uint32_t bitmap_t;

void bitmap_set (bitmap_t* map, int bit);
void bitmap_unset (bitmap_t* map, int bit);
bool bitmap_test (bitmap_t* map, int bit);

#endif