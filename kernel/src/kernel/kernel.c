#include <libk/stdlib.h>
#include <dev/vga.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/isr.h>
#include <arch/irq.h>
#include <dev/timer.h>
#include <dev/keyboard.h>
#include <boot/multiboot.h>
#include <mem/pmm.h>
#include <mem/vmm.h>

#ifdef DEBUG
#include <kernel/debug.h>
#endif

extern int end;

int k_main(struct multiboot_info* mboot)
{
	monitor_clear();
	monitor_write("Welcome to Kernel!\n");

	#ifdef DEBUG_VGA
	monitor_write("New lines work!\n");
	monitor_set_fg_color(LCYAN);
	monitor_set_bg_color(MAGENTA);

	monitor_write("Colors work too!\n");
	monitor_set_fg_color(WHITE);
	monitor_set_bg_color(BLACK);
	#endif

	gdt_install();
	#ifdef DEBUG_GDT
	monitor_write("Segments work too!\n");
	#endif

	idt_install();
	#ifdef DEBUG_IDT
	monitor_write("Interrupt descriptors work too!\n");
	#endif

	isr_install();
	#ifdef DEBUG_ISR
	monitor_write("Exceptions work too!\n");
	#endif

	irq_install();
	#ifdef DEBUG_IRQ
	monitor_write("Interrupts work too!\n");
	#endif

	timer_install();
	#ifdef DEBUG_PIC
	monitor_write("Timer works too!\n");
	#endif

	keyboard_install();
	#ifdef DEBUG_KBD
	monitor_write("Keyboard works too!\n");
	#endif

	#ifdef DEBUG_BIOS_MMAP
	monitor_write("Memory Map Address: ");
	monitor_write_hex(mboot->mmap_addr);
	monitor_write("\n");

	monitor_write("Memory Map Length: ");
	monitor_write_hex(mboot->mmap_length);
	monitor_write("\n");
	#endif

	u32int_t memSize = 1024 + mboot->mem_lower + mboot->mem_upper * 64;

	#ifdef DEBUG_BIOS_MMAP
	monitor_write("Memory size: ");
	monitor_write_hex(memSize);
	monitor_write("\n");
	#endif

	pmmngr_init (memSize, &end + 1);

	multiboot_memory_map_t* mmap = mboot->mmap_addr;
	while(mmap < mboot->mmap_addr + mboot->mmap_length)
	{
		#ifdef DEBUG_BIOS_MMAP
		monitor_write("Memory Area\n");

		monitor_write("Start:");
		monitor_write_hex(mmap->addr);
		monitor_write(" ");

		monitor_write("End: ");
		monitor_write_hex(mmap->addr + mmap->len);
		monitor_write(" ");

		monitor_write("Type:");
		monitor_write_hex(mmap->type);
		monitor_write("\n");
		#endif

		if(mmap->type == 1)
		{
			pmmngr_init_region (mmap->addr, mmap->len);
		}

		mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
	}

	// Mark kernel as used!
	pmmngr_deinit_region (0x100000, &end - 0x100000);

	u32int_t total_blocks = pmmngr_get_block_count();
	u32int_t used_blocks = pmmngr_get_use_block_count();
	u32int_t free_blocks = pmmngr_get_free_block_count();

	#ifdef DEBUG_PMM
	monitor_write("Total blocks: ");
	monitor_write_hex(total_blocks);
	monitor_write("\n");

	monitor_write("Used blocks: ");
	monitor_write_hex(used_blocks);
	monitor_write("\n");

	monitor_write("Free blocks: ");
	monitor_write_hex(free_blocks);
	monitor_write("\n");

	monitor_write("Physical Memory works too!\n");
	#endif

	/*
	monitor_write("\n");

	monitor_write("Allocating an unsigned int.\n");
	monitor_write("Address: ");
	u32int_t* p = (u32int_t*)pmmngr_alloc_block ();
	monitor_write_hex(&p);
	monitor_write("\n");

	monitor_write("\n");

	monitor_write("Allocating another unsigned int.\n");
	monitor_write("Address: ");
	u32int_t* p2 = (u32int_t*)pmmngr_alloc_block ();
	monitor_write_hex(&p2);
	monitor_write("\n");

	monitor_write("\n");

	monitor_write("Assigning value '2' to the first one.\n");
	*p = 2;

	monitor_write("\n");

	monitor_write("Assigning value '4' to the first one.\n");
	*p2 = 4;

	monitor_write("\n");

	monitor_write("Value of first: ");
	monitor_write_hex(*p);
	monitor_write("\n");

	monitor_write("Value of second: ");
	monitor_write_hex(*p2);
	monitor_write("\n");

	monitor_write("\n");

	monitor_write("Freeing first!\n");
	pmmngr_free_block(p);
	monitor_write("Freeing second!\n");
	pmmngr_free_block(p2);
	monitor_write("Done!\n");
	*/

	vmmngr_initialize ();

	#ifdef DEBUG_VMM
	monitor_write("Virtual Memory works too!\n");

	monitor_write("k_main() address:");
	monitor_write_hex(k_main);
	monitor_write("\n");

	#endif

	// For testing assertions.
	// ASSERT(1 == 0xbadb002);

	// For testing stack traces.
	// kstrace(10);

	// For testing exceptions.
	// __asm__ __volatile__("int $6");
	// timer_wait(1000);
	// int zero = 0xFFFFFFFF/0;

	return 0xDEADBEEF;
}
