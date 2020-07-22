#include <mem/vmm.h>
#include <mem/pmm.h>
#include <libk/dbg.h>
#include <libk/types.h>
#include <libk/string.h>
#include <task/spinlock.h>

// Page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// Directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// Page size is 4k
#define PAGE_SIZE 4096

// Current directory table
pdirectory* _cur_directory = 0;

// Current page directory base register
physical_addr _cur_pdbr = 0;

// Kernel page directory base register
physical_addr kernel_pdbr = 0;

// Virtual allocator spinlock
spinlock_t vmm_lock = {.lock = 0};

pt_entry* vmmngr_ptable_lookup_entry (ptable* p,virtual_addr addr)
{
	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return 0;
}

pd_entry* vmmngr_pdirectory_lookup_entry (pdirectory* p, virtual_addr addr)
{
	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return 0;
}

int vmmngr_switch_pdirectory (pdirectory* dir)
{
	if (!dir)
		return 0;

	_cur_directory = dir;
	pmm_load_PDBR (_cur_pdbr);
	return 1;
}

void vmmngr_flush_tlb_entry (virtual_addr addr)
{
   __asm__ volatile("cli");
   __asm__ volatile("invlpg (%0)" :: "r"(addr) : "memory");
   __asm__ volatile("sti");
}

pdirectory* vmmngr_get_directory ()
{
   return _cur_directory;
}

int vmmngr_alloc_page (pt_entry* e)
{
   acquire_spinlock(&vmm_lock);

	// Allocate a free physical frame
	void* p = pmm_alloc_block ();
	if (!p)
		return 0;

	// Map it to the page
	pt_entry_set_frame (e, (physical_addr)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);
	pt_entry_add_attrib (e, I86_PTE_WRITABLE);
	// does set WRITE flag!

   release_spinlock(&vmm_lock);

	return 1;
}

void vmmngr_free_page (pt_entry* e)
{
   acquire_spinlock(&vmm_lock);

	void* p = (void*)pt_entry_pfn (*e);
	if (p)
		pmm_free_block (p);

	pt_entry_del_attrib (e, I86_PTE_PRESENT);
   pt_entry_del_attrib (e, I86_PTE_WRITABLE);

   release_spinlock(&vmm_lock);
}

void vmmngr_map_page (void* phys, void* virt)
{
   acquire_spinlock(&vmm_lock);

   dbg("[VMM] Mapping %x -> %x\n", phys, virt);

   // Get page directory
   pdirectory* pageDirectory = vmmngr_get_directory ();

   // Get page table
   pd_entry* e = &pageDirectory->m_entries [PAGE_DIRECTORY_INDEX ((uint32_t) virt) ];
   if ( (*e & I86_PTE_PRESENT) != I86_PTE_PRESENT)
   {
      // Page table not present, allocate it
      ptable* table = (ptable*) pmm_alloc_block ();
      if (!table)
         return;

      // Clear page table
      memset (table, 0, sizeof(ptable));

      // Create a new entry
      pd_entry* entry =
         &pageDirectory->m_entries [PAGE_DIRECTORY_INDEX ((uint32_t) virt)];

      // Map in the table (Can also just do *entry |= 3) to enable these bits
      pd_entry_add_attrib (entry, I86_PDE_PRESENT);
      pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
      pd_entry_set_frame (entry, (physical_addr)table);
   }

   // Get table
   ptable* table = (ptable*) PAGE_GET_PHYSICAL_ADDRESS ( e );

   // Get page
   pt_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt)];

   // Map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame ( page, (physical_addr) phys);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
   pt_entry_add_attrib ( page, I86_PTE_WRITABLE);

   release_spinlock(&vmm_lock);
}

void vmmngr_initialize ()
{
   // Allocate default page table
   ptable* table = (ptable*) pmm_alloc_block ();
   if (!table)
   {
      kpanic("Failed to allocate ptable!", 0);
      return;
   }

   // Allocates 3gb page table
   ptable* table2 = (ptable*) pmm_alloc_block ();
   if (!table2)
   {
      kpanic("Failed to allocate ptable!", 0);
      return;
   }

   // Clear page table
   memset (table, 0, sizeof (ptable));

   // 1st 4mb are idenitity mapped
   for (int i = 0, frame=0x0, virt=0x00000000; i < 1024; i++, frame += 4096, virt += 4096)
   {
	dbg("[VMM] Mapping %x -> %x\n", frame, virt);
      // Create a new page
      pt_entry page = 0;
      pt_entry_add_attrib (&page, I86_PTE_PRESENT);
      pt_entry_add_attrib (&page, I86_PTE_WRITABLE);
      pt_entry_set_frame (&page, frame);

      // ...and add it to the page table
      table2->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
   }

   // Map 1mb to 3gb (where we are at)
   for (int i = 0, frame=0x100000, virt=0xc0000000; i < 1024; i++, frame += 4096, virt += 4096)
   {
	dbg("[VMM] Mapping %x -> %x\n", frame, virt);
      // Create a new page
      pt_entry page = 0;
      pt_entry_add_attrib (&page, I86_PTE_PRESENT);
      pt_entry_add_attrib (&page, I86_PTE_WRITABLE);
      pt_entry_set_frame (&page, frame);

      // ...and add it to the page table
      table->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
   }

   // Create default directory table
   pdirectory* dir = (pdirectory*) pmm_alloc_blocks (3);
   if (!dir)
   {
      kpanic("Failed to allocate pdir!", 0);
      return;
   }

  // Clear directory table and set it as current
  memset (dir, 0, sizeof (pdirectory));

   // Get first entry in dir table and set it up to point to our table
   pd_entry* entry = &dir->m_entries [PAGE_DIRECTORY_INDEX (0xc0000000) ];
   pd_entry_add_attrib (entry, I86_PDE_PRESENT);
   pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
   pd_entry_set_frame (entry, (physical_addr)table);

   pd_entry* entry2 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
   pd_entry_add_attrib (entry2, I86_PDE_PRESENT);
   pd_entry_add_attrib (entry2, I86_PDE_WRITABLE);
   pd_entry_set_frame (entry2, (physical_addr)table2);

   // Store current PDBR
   _cur_pdbr = (physical_addr) &dir->m_entries;

   // Also set the kernel PDBR
   kernel_pdbr = _cur_pdbr;

   // Switch to our page directory
   vmmngr_switch_pdirectory (dir);

   // Enable paging
   pmm_paging_enable (1);
}
