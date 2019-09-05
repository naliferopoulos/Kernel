#include <ds/bitmap.h>

void bitmap_set (bitmap_t* map, int bit)
{
	map[bit / 32] |= (1 << (bit % 32));
}

void bitmap_unset (bitmap_t* map, int bit)
{
	map[bit / 32] &= ~ (1 << (bit % 32));
}

bool bitmap_test (bitmap_t* map, int bit)
{
	return map[bit / 32] &  (1 << (bit % 32));
}