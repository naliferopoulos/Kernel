#ifndef VMM_H
#define VMM_H

#include <libk/types.h>
#include <mem/pte.h>
#include <mem/pde.h>

//! virtual address
typedef uint32_t virtual_addr;

//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

// Page table
typedef struct ptable
{
	pt_entry m_entries[PAGES_PER_TABLE];
} ptable;

// Page directory
typedef struct pdirectory
{
	pd_entry m_entries[PAGES_PER_DIR];
} pdirectory;

// Initialize the memory manager
void vmmngr_initialize ();

// Allocates a page in physical memory
int vmmngr_alloc_page (pt_entry*);

// Frees a page in physical memory
void vmmngr_free_page (pt_entry* e);

// Switch to a new page directory
int vmmngr_switch_pdirectory (pdirectory*);

// Get current page directory
pdirectory* vmmngr_get_directory ();

// Flushes a cached translation lookaside buffer (TLB) entry
void vmmngr_flush_tlb_entry (virtual_addr addr);

// Clears a page table
void vmmngr_ptable_clear (ptable* p);

// Convert virtual address to page table index
uint32_t vmmngr_ptable_virt_to_index (virtual_addr addr);

// Get page entry from page table
pt_entry* vmmngr_ptable_lookup_entry (ptable* p, virtual_addr addr);

// Convert virtual address to page directory index
uint32_t vmmngr_pdirectory_virt_to_index (virtual_addr addr);

// Clears a page directory table
void vmmngr_pdirectory_clear (pdirectory* dir);

// Maps a physical to a virtual page
void vmmngr_map_page (void* phys, void* virt);

// Get directory entry from directory table
pd_entry* vmmngr_pdirectory_lookup_entry (pdirectory* p, virtual_addr addr);

#endif
