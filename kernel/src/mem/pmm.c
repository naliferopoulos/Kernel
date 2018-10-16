#include <mem/pmm.h>
#include <libk/stdlib.h>

// 8 blocks per byte
#define BLOCKS_PER_BYTE 8

// Block size (4k)
#define BLOCK_SIZE	4096

// Block alignment
#define BLOCK_ALIGN	BLOCK_SIZE

// Size of physical memory
u32int_t mem_size = 0;

// Number of blocks currently in use
u32int_t used_blocks = 0;

// Maximum number of available memory blocks
u32int_t max_blocks = 0;

// Memory map bit array. Each bit represents a memory block
u32int_t* mem_map = 0;

// Enable paging!
extern void paging_enable();

// Disable paging!
extern void paging_disable();

extern physical_addr pmmngr_get_PDBR();

// Set any bit (frame) within the memory map bit array
void mmap_set (int bit)
{
	mem_map[bit / 32] |= (1 << (bit % 32));
}

// Unset any bit (frame) within the memory map bit array
void mmap_unset (int bit)
{
	mem_map[bit / 32] &= ~ (1 << (bit % 32));
}

// Test if any bit (frame) is set within the memory map bit array
int mmap_test (int bit)
{
	return mem_map[bit / 32] &  (1 << (bit % 32));
}

// Finds first free frame in the bit array and returns its index
int mmap_first_free ()
{
	// Find the first free bit
	for (u32int_t i=0; i< pmmngr_get_block_count() /32; i++)
		if (mem_map[i] != 0xffffffff)
			for (int j=0; j<32; j++)
			{				// Test each bit in the dword
				int bit = 1 << j;
				if (!(mem_map[i] & bit))
					return i*4*8+j;
			}

	return -1;
}

// Finds first free "size" number of frames and returns its index
int mmap_first_free_s (size_t size)
{
	if (size==0)
		return -1;

	if (size==1)
		return mmap_first_free ();

	for (u32int_t i=0; i<pmmngr_get_block_count() /32; i++)
		if (mem_map[i] != 0xffffffff)
			for (int j=0; j<32; j++)
			{	// Test each bit in the dword
				int bit = 1<<j;
				if (! (mem_map[i] & bit) )
				{
					int startingBit = i*32;
					startingBit+=bit;		// Get the free bit in the dword at index i

					u32int_t free=0; // Loop through each bit to see if its enough space
					for (u32int_t count=0; count<=size;count++)
					{
						if (!mmap_test (startingBit+count))
							free++;	// This bit is clear (free frame)

						if (free==size)
							return i*4*8+j; // free count==size needed; return index
					}
				}
			}

	return -1;
}

void pmmngr_init (size_t memSize, physical_addr bitmap)
{
	mem_size = memSize;
	mem_map	= (u32int_t*) bitmap;
	max_blocks	= (pmmngr_get_memory_size()*1024) / BLOCK_SIZE;
	used_blocks	= max_blocks;

	// By default, all of memory is in use
	kmemset (mem_map, 0xf, pmmngr_get_block_count() / BLOCKS_PER_BYTE );
}

void pmmngr_init_region (physical_addr base, size_t size)
{
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks>=0; blocks--)
	{
		mmap_unset (align++);
		used_blocks--;
	}

	mmap_set (0);	// First block is always set. This insures allocs cant be 0.
}

void pmmngr_deinit_region (physical_addr base, size_t size)
{
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks>=0; blocks--)
	{
		mmap_set (align++);
		used_blocks++;
	}
}

void* pmmngr_alloc_block ()
{
	if (pmmngr_get_free_block_count() <= 0)
		return 0;	// Out of memory

	int frame = mmap_first_free ();

	if (frame == -1)
		return 0;	// Out of memory

	mmap_set (frame);

	physical_addr addr = frame * BLOCK_SIZE;
	used_blocks++;

	return (void*)addr;
}

void pmmngr_free_block (void* p)
{
	physical_addr addr = (physical_addr)p;
	int frame = addr / BLOCK_SIZE;

	mmap_unset (frame);

	used_blocks--;
}

void* pmmngr_alloc_blocks (size_t size)
{
	if (pmmngr_get_free_block_count() <= size)
		return 0;	// Not enough space

	int frame = mmap_first_free_s (size);

	if (frame == -1)
		return 0;	// Not enough space

	for (u32int_t i = 0; i < size; i++)
		mmap_set (frame+i);

	physical_addr addr = frame * BLOCK_SIZE;
	used_blocks+=size;

	return (void*)addr;
}

void pmmngr_free_blocks (void* p, size_t size)
{
	physical_addr addr = (physical_addr)p;
	int frame = addr / BLOCK_SIZE;

	for (u32int_t i=0; i<size; i++)
		mmap_unset (frame+i);

	used_blocks-=size;
}

size_t pmmngr_get_memory_size ()
{
	return mem_size;
}

u32int_t pmmngr_get_block_count ()
{
	return max_blocks;
}

u32int_t pmmngr_get_use_block_count ()
{
	return used_blocks;
}

u32int_t pmmngr_get_free_block_count ()
{
	return max_blocks - used_blocks;
}

u32int_t pmmngr_get_block_size ()
{
	return BLOCK_SIZE;
}

void pmmngr_paging_enable (int b)
{
	if(b == 0)
		paging_disable();
	else
		paging_enable();
}

// Load cr3
void pmmngr_load_PDBR(physical_addr addr)
{
	__asm__ volatile("mov %0, %%cr3":: "r"(addr));
}
