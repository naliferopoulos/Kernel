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
#include <libk/stdio.h>

#ifdef DEBUG
#include <kernel/debug.h>
#endif

extern int end;

int k_main(struct multiboot_info* mboot)
{
	monitor_clear();
	printf("Welcome to Kernel!\n");

	#ifdef DEBUG_VGA
	printf("New lines work!\n");
	monitor_set_fg_color(LCYAN);
	monitor_set_bg_color(MAGENTA);

	printf("Colors work too!\n");
	monitor_set_fg_color(WHITE);
	monitor_set_bg_color(BLACK);
	#endif

	gdt_install();
	#ifdef DEBUG_GDT
	printf("Segments work too!\n");
	#endif

	idt_install();
	#ifdef DEBUG_IDT
	printf("Interrupt descriptors work too!\n");
	#endif

	isr_install();
	#ifdef DEBUG_ISR
	printf("Exceptions work too!\n");
	#endif

	irq_install();
	#ifdef DEBUG_IRQ
	printf("Interrupts work too!\n");
	#endif

	timer_install();
	#ifdef DEBUG_PIC
	printf("Timer works too!\n");
	#endif

	keyboard_install();
	#ifdef DEBUG_KBD
	printf("Keyboard works too!\n");
	#endif

	#ifdef DEBUG_BIOS_MMAP
	monitor_write("Memory Map Address: ");
	monitor_write_hex(mboot->mmap_addr);
	monitor_write("\n");

	monitor_write("Memory Map Length: ");
	monitor_write_hex(mboot->mmap_length);
	monitor_write("\n");
	#endif

	uint32_t memSize = 1024 + mboot->mem_lower + mboot->mem_upper * 64;

	#ifdef DEBUG_BIOS_MMAP
	char memString[10];
	utoa(memSize, memString, 10);

	printf("Memory size: %s\n", memString);
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

	uint32_t total_blocks = pmmngr_get_block_count();
	uint32_t used_blocks = pmmngr_get_use_block_count();
	uint32_t free_blocks = pmmngr_get_free_block_count();

	char total_blocks_string[10];
	char used_blocks_string[10];
	char free_blocks_string[10];

	// Sanity Check
	ASSERT(total_blocks == (used_blocks + free_blocks));

	utoa(total_blocks, total_blocks_string, 10);
	utoa(used_blocks, used_blocks_string, 10);
	utoa(free_blocks, free_blocks_string, 10);

	#ifdef DEBUG_PMM
	printf("Total blocks: %s\n", total_blocks_string);
	printf("Used blocks: %s\n", used_blocks_string);
	printf("Free blocks: %s\n", free_blocks_string);

	printf("Physical Memory works too!\n");
	#endif

	vmmngr_initialize ();

	#ifdef DEBUG_VMM
	printf("Virtual Memory works too!\n");

	char k_main_address[10];
	printf("k_main() address:");
	utoa(k_main, k_main_address, 16);
	printf("0x%s\n", k_main_address);

	#endif

	// For testing assertions.
	ASSERT("That's it for now!" == 0);

	// For testing stack traces.
	// kstrace(10);

	// For testing exceptions.
	// __asm__ __volatile__("int $6");
	// timer_wait(1000);
	// int zero = 0xFFFFFFFF/0;

	return 0xDEADBEEF;
}
