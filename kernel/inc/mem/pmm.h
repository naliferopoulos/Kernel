#ifndef PMM_H
#define PMM_H

#include <libk/types.h>

// Physical address
typedef	uint32_t physical_addr;

// Size type
typedef uint32_t size_t;

void mmap_set (int bit);
void mmap_unset (int bit);
int mmap_test (int bit);

// Initialize the physical memory manager
void pmmngr_init (size_t, physical_addr);

// Enables a physical memory region for use
void pmmngr_init_region (physical_addr, size_t);

// Disables a physical memory region as in use (unuseable)
void pmmngr_deinit_region (physical_addr base, size_t);

// Allocates a single memory block
void* pmmngr_alloc_block ();

// Releases a memory block
void pmmngr_free_block (void*);

// Allocates blocks of memory
void* pmmngr_alloc_blocks (size_t);

// Frees blocks of memory
void pmmngr_free_blocks (void*, size_t);

// Returns amount of physical memory the manager is set to use
size_t pmmngr_get_memory_size ();

// Returns number of blocks currently in use
uint32_t pmmngr_get_use_block_count ();

// Returns number of blocks not in use
uint32_t pmmngr_get_free_block_count ();

// Returns number of memory blocks
uint32_t pmmngr_get_block_count ();

// Returns default memory block size in bytes
uint32_t pmmngr_get_block_size ();

// Enable or disable paging
void pmmngr_paging_enable (int enabled);

// Loads the page directory base register (PDBR)
void pmmngr_load_PDBR (physical_addr);

// Get PDBR physical address
physical_addr pmmngr_get_PDBR ();

#endif
