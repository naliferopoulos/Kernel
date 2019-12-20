#include <mem/pmm.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <libk/types.h>
#include <task/spinlock.h>
#include <ds/bitmap.h>
#include <libk/dbg.h>

// 8 blocks per byte
#define BLOCKS_PER_BYTE 8

// Block size (4k)
#define BLOCK_SIZE	4096

// Block alignment
#define BLOCK_ALIGN	BLOCK_SIZE

// Size of physical memory (For knowing the max available RAM)
uint32_t mem_size = 0;

// Number of blocks currently free. (For statistics)
uint32_t free_blocks = 0;

// Number of available blocks. (The maximum number of blocks we can use).
uint32_t avail_blocks = 0;

// Number of used blocks can be calculated using the following:
// used = avail - free;

// Maximum number of memory blocks (We need a bitmap that can hold info regarding all of these!)
uint32_t max_blocks = 0;

// Memory bitmap. Each bit represents a memory block
bitmap_t* mem_map = 0;

// Frame allocator spinlock
spinlock_t pmm_lock = {.lock = 0};

// Enable paging!
extern void paging_enable();

// Disable paging!
extern void paging_disable();

extern physical_addr pmm_get_PDBR();

uint32_t align_to_upper(uint32_t a)
{
	return (a & (PAGE_SIZE-1)) ? ((a+PAGE_SIZE) & ~(PAGE_SIZE-1)) : a;
}

// Finds first free frame in the bit array and returns its index
static int pmm_first_free()
{
	// Find the first free bit
	for (uint32_t i=0; i< max_blocks / 32; i++)
		if (mem_map[i] != 0xffffffff)
			for (int j=0; j<32; j++)
			{	
				// Test each bit in the dword
				int bit = 1 << j;
				if (!(mem_map[i] & bit))
					return i*4*8+j;
			}
	return -1;
}

static int pmm_first_free_s(size_t s)
{
	// Make sure the requested size is sane.
	ASSERT(s > 0);
	
	// If we only request one block, we already have an implementation.
	if(s == 1)
		return pmm_first_free();

	for (uint32_t i = 0; i < max_blocks / 32; i++)
	{
		if (mem_map[i] != 0xffffffff)
		{
			for (int j = 0; j < 32; j++)
			{
				int bit = 1<<j;
				if (! (mem_map[i] & bit) )
				{
					int startingBit = i*32;
					startingBit+=bit; // Get the free bit in the dword at index i

					uint32_t free=0; // Loop through each bit to see if its enough space
					for (uint32_t count=0; count <= s;count++)
					{
						if (!bitmap_test(mem_map, startingBit+count))
							free++;

						if(free == s)
							return i*32 + j;
					}
				}
			}
		}
	}

	return -1;
}

void pmm_init (size_t memSize, physical_addr bitmap)
{
	dbg("[PMM] Initializing memory bitmap at %x\n", bitmap);
	
	mem_size = memSize;
	mem_map	= (bitmap_t*) bitmap;
	//max_blocks = (memSize * 1024) / BLOCK_SIZE;
	max_blocks = memSize / BLOCK_SIZE;
	free_blocks = 0;
	avail_blocks = 0;

	dbg("[PMM] Splitting %x into %d blocks.\n", mem_size, max_blocks);

	// By default, all of memory is in use
	memset (mem_map, 0xfffffff, max_blocks / BLOCKS_PER_BYTE );

	dbg("[PMM] %d/%d free blocks.\n", free_blocks, avail_blocks);
}

// NOTE: pmm_init_region and pmm_deinit_region are expected to only be called before tasking is initialized.
// Due to that, they are not implemented to be atomic!

void pmm_init_region (physical_addr base, size_t size)
{
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks>=0; blocks--)
	{
		bitmap_unset(mem_map, align++);
		avail_blocks++;
	}

	bitmap_set(mem_map, 0);	// First block is always set. This ensures allocs cant be 0.
	
	dbg("[PMM] Initialized PMM region %x --> %x\n", base, base + size);
	dbg("[PMM] %d/%d free blocks.\n", free_blocks, avail_blocks);
}

void pmm_deinit_region (physical_addr base, size_t size)
{
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks>=0; blocks--)
	{
		bitmap_set(mem_map, align++);
		avail_blocks--;
	}

	dbg("[PMM] Deinitialized PMM region %x --> %x\n", base, base + size);
	dbg("[PMM] %d/%d free blocks.\n", free_blocks, avail_blocks);
}

void pmm_finalize ()
{
	// Deinit the bitmap itself!
	pmm_deinit_region((physical_addr) mem_map, align_to_upper(max_blocks / BLOCKS_PER_BYTE));

	// By default all available blocks are free.
	free_blocks = avail_blocks;
	
	dbg("[PMM] Physical memory finalized with bitmap %x --> %x\n", mem_map, mem_map + max_blocks / BLOCKS_PER_BYTE);
	dbg("[PMM] %d/%d free blocks.\n", free_blocks, avail_blocks);
}

void* pmm_alloc_block ()
{
	acquire_spinlock(&pmm_lock);

	ASSERT(free_blocks > 0);

	int frame = pmm_first_free ();

	ASSERT(frame != -1);

	bitmap_set(mem_map, frame);

	physical_addr addr = frame * BLOCK_SIZE;
	free_blocks--;

	dbg("[PMM] Allocating block at %x\n", addr);

	release_spinlock(&pmm_lock);

	return (void*)addr;
}

void pmm_free_block (void* p)
{
	acquire_spinlock(&pmm_lock);

	physical_addr addr = (physical_addr)p;
	int frame = addr / BLOCK_SIZE;

	bitmap_unset(mem_map, frame);

	free_blocks++;
	
	dbg("[PMM] Freeing block at %x\n", p);

	release_spinlock(&pmm_lock);
}

void* pmm_alloc_blocks (size_t size)
{
	acquire_spinlock(&pmm_lock);

	// Make sure we have enough free space.
	ASSERT(free_blocks >= size);

	int frame = pmm_first_free_s (size);

	if (frame == -1)
		return 0;	// Not enough space

	for (uint32_t i = 0; i < size; i++)
		bitmap_set(mem_map, frame+i);

	physical_addr addr = frame * BLOCK_SIZE;
	free_blocks -= size;

	release_spinlock(&pmm_lock);

	return (void*)addr;
}

void pmm_free_blocks (void* p, size_t size)
{
	acquire_spinlock(&pmm_lock);

	physical_addr addr = (physical_addr)p;
	int frame = addr / BLOCK_SIZE;

	for (uint32_t i=0; i<size; i++)
		bitmap_unset(mem_map, frame+i);

	free_blocks += size;

	release_spinlock(&pmm_lock);
}

size_t pmm_get_memory_size ()
{
	return mem_size;
}

void pmm_paging_enable (int b)
{
	if(b == 0)
		paging_disable();
	else
		paging_enable();
}

// Load cr3
void pmm_load_PDBR(physical_addr addr)
{
	__asm__ volatile("mov %0, %%cr3":: "r"(addr));
}
