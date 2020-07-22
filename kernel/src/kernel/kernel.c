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
#include <mem/heap.h>
#include <libk/stdio.h>
#include <fs/fs.h>
#include <dev/initrd.h>
#include <dev/serial.h>
#include <libk/dbg.h>
#include <dev/cpuid.h>
#include <dev/cmos.h>
#include <task/process.h>

#ifdef DEBUG
#include <kernel/debug.h>
#endif

extern uint32_t end;
//extern fs_node_t* fs_root;

int k_main(struct multiboot_info* mboot, uint32_t magic)
{
	// Before proceeding any further, make sure we were booted by a multiboot compliant bootloader!
	ASSERT(magic == MULTIBOOT_BOOTLOADER_MAGIC);

	
	setup_serial();
	#ifdef DEBUG_SERIAL
	dbg("[+] Serial driver active.\n");
	#endif

	dbg("[+] Kernel early log device initialized!\n");

	dbg("[+] Processor: %s\n", cpuid_name());

	monitor_clear();
	monitor_set_bg_color(RED);
	monitor_set_fg_color(YELLOW);
	monitor_write_center("");

	monitor_write_center("Welcome to Kernel!");
	monitor_write_center("");
	
	monitor_write_center("Git Commit ID: " GIT);
	monitor_write_center("GCC Version: " GCC_VERSION " " GCC_TARGET);

	monitor_write_center(cpuid_name());

	if(mboot->boot_loader_name != 0)
		monitor_write_center((char *)mboot->boot_loader_name);
	else
		monitor_write_center("Unknown Bootloader!");
	
	monitor_write_center("");
	monitor_set_bg_color(BLACK);
	monitor_set_fg_color(WHITE);
	monitor_write_center("");

	// Ensure modules where successfully loaded!
	ASSERT(CHECK_MBOOT_FLAG(mboot->flags, MOD_FLAG));

	// Before proceeding any further, make sure we were passed an initial ram disk and a symbol map!
	ASSERT(mboot->mods_count == 2);

	multiboot_module_t *mod = (multiboot_module_t*)(mboot->mods_addr);
	uint32_t initrd_location = (uint32_t)(mod->mod_start);
	uint32_t initrd_end = (uint32_t)(mod->mod_end);

	// Fetch the next module.
	mod++;

	uint32_t symmap_location = (uint32_t)(mod->mod_start);
	uint32_t symmap_end = (uint32_t)(mod->mod_end);

	set_symbol_map(symmap_location);

	#ifdef DEBUG_MODULES
	dbg("\tMultiboot Module Count: %d\n", mboot->mods_count);
	dbg("\tMultiboot Module Address: %x\n", mboot->mods_addr);
	#endif
	
	#ifdef DEBUG_RD
	dbg("[+] Ramdisk\n");
	dbg("\tRamdisk Location: %x --> %x\n", initrd_location, initrd_end);
	#endif

	#ifdef DEBUG_SYMBOLS
	dbg("[+] Symbol Map\n");
	dbg("\tSymmap Location: %x --> %x\n", symmap_location, symmap_end);
	#endif

	#ifdef DEBUG_VGA
	dbg("[+] Graphics driver active.\n");
	#endif

	gdt_install();
	#ifdef DEBUG_GDT
	dbg("[+] Segmentation active.\n");
	#endif

	idt_install();
	#ifdef DEBUG_IDT
	dbg("[+] Interrupt descriptors active.\n");
	#endif

	isr_install();
	#ifdef DEBUG_ISR
	dbg("[+] Interrupt service request handlers active.\n");
	#endif

	irq_install();
	#ifdef DEBUG_IRQ
	dbg("[+] Interrupt request handlers active.\n");
	#endif

	timer_install();
	#ifdef DEBUG_PIC
	dbg("[+] Timer active.\n");
	#endif

	keyboard_install();
	#ifdef DEBUG_KBD
	dbg("[+] Keyboard driver active.\n");
	#endif

	// Ensure a memory map was passed!
	ASSERT(CHECK_MBOOT_FLAG(mboot->flags, MMAP_FLAG));

	#ifdef DEBUG_BIOS_MMAP
	dbg("\tMemory Map Address: %x, Memory Map Length: %x\n", mboot->mmap_addr, mboot->mmap_length);
	#endif

	// Parse the memory map to identify the sum of every memory area pool size.
	uint32_t memSize = 0;
	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mboot->mmap_addr;
	while(mmap < (multiboot_memory_map_t*)(mboot->mmap_addr + mboot->mmap_length))
	{
		memSize += mmap->len_low;
		mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
	}

	//uint32_t memSize = mboot->mem_upper + mboot->mem_lower;//1024 + mboot->mem_lower + mboot->mem_upper * 64;

	#ifdef DEBUG_BIOS_MMAP
	dbg("\tMemory size: %x\n", memSize);
	#endif

	// Initialize the physical memory manager with the total memory size, and the first free memory block used as a bitmap.
	//pmm_init (memSize, (int)(&end + 1));
	//pmm_init (memSize, (int)(initrd_end + 1));
	pmm_init(memSize, (size_t)(align_to_upper(symmap_end)));

	mmap = (multiboot_memory_map_t*)mboot->mmap_addr;
	while(mmap < (multiboot_memory_map_t*)(mboot->mmap_addr + mboot->mmap_length))
	{
		#ifdef DEBUG_BIOS_MMAP
		dbg("\tMemory Area -> Start: %x, Length: %x, Type: %x\n", mmap->addr_low, mmap->len_low, mmap->type);
		#endif

		if(mmap->type == 1)
		{
			// Only start allocations above 0. Makes things easier to debug. (It skips over the usual sub-1MB RAM pool. 
			if(mmap->addr_low > 0)
				pmm_init_region (mmap->addr_low, mmap->len_low);
		}

		mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(mmap->size));
	}

	// Mark kernel and modules as used!
	pmm_deinit_region (0x100000, align_to_upper(symmap_end) - 0x100000);

	// Finalize the memory map.
	pmm_finalize();

	#ifdef DEBUG_PMM
	dbg("[+] Physical memory manager active.\n");
	#endif

	vmmngr_initialize ();
	#ifdef DEBUG_VMM
	dbg("[+] Virtual memory manager active.\n");
	#endif

	// Pause. We NEED to identity map all kernel modules. 
	// Remember, their starting address is always page aligned, their ending address is not.
	
	// Start with the ramdisk.
	//uint32_t initrd_end_aligned = align_to_upper(initrd_end); 
	//for(int i = initrd_location; i < initrd_end_aligned; i+=4096)
	//	vmmngr_map_page(i, i);

	// Then identity map the symbol map.
	//uint32_t symmap_end_aligned = align_to_upper(symmap_end);
	//for(int i = symmap_location; i < symmap_end_aligned; i+=4096)
	//	vmmngr_map_page(i, i);
		
	// From now on, the heap can be used!
	#ifdef DEBUG_VMM
	dbg("Attempting allocations to test the kernel heap...\n");

	dbg("Allocating ptr1 with 20 bytes.\n");
	void* ptr1 = kmalloc(20);
	dbg("\tptr1: %x\n", ptr1);

	dbg("Allocating ptr2 with 40 bytes.\n");
	void* ptr2 = kmalloc(40);
	dbg("\tptr2: %x\n", ptr2);

	dbg("Allocating ptr3 with 20 bytes.\n");
	void* ptr3 = kmalloc(20);
	dbg("\tptr3: %x\n", ptr3);
	
	dbg("Freeing ptr2.\n");
	kfree(ptr2);

	dbg("Allocating ptr4 with 20 bytes.\n");
	void* ptr4 = kmalloc(20);
	dbg("\tptr4: %x\n", ptr4);
	
	dbg("Freeing ptr4.\n");
	kfree(ptr4);

	dbg("Freeing ptr1.\n");
	kfree(ptr1);

	dbg("Freeing ptr3.\n");
	kfree(ptr3);
	
	dbg("Allocating ptr5 with 20 bytes.\n");
	void* ptr5 = kmalloc(20);
	dbg("\tptr5: %x\n", ptr5);

	dbg("Freeing ptr5.\n");
	kfree(ptr5);

	dbg("Allocating ptr6 with 4095 bytes.\n");
	void* ptr6 = kmalloc(4095);
	dbg("\tptr6: %x\n", ptr6);

	dbg("Allocating ptr7 with 4096 bytes.\n");
	void* ptr7 = kmalloc(4096);
	dbg("\tptr7: %x\n", ptr7);

	dbg("Freeing ptr6.\n");
	kfree(ptr6);

	dbg("Freeing ptr7.\n");
	kfree(ptr7);

	dbg("Allocating ptr8 with 20 bytes.\n");
	void* ptr8 = kmalloc(20);
	dbg("\tptr8: %x\n", ptr8);

	dbg("Freeing ptr8.\n");
	kfree(ptr8);

	//dbg("Allocating ptr9 with 0xFFFF0000 bytes.\n");
	//void* ptr9 = kmalloc(0xFFFF0000);
	//dbg("\tptr9: %x\n", ptr9);

	// To test double free.
	//void* ptr10 = kmalloc(20);
	//kfree(ptr10);
	//kfree(ptr10);

	#endif

	// Initialize the root file system to be the ramdisk.
	fs_root = initialise_initrd(initrd_location);

	ASSERT(fs_root != 0);

	#ifdef DEBUG_RD
	dbg("[+] Ramdisk driver active.\n");
	dbg("\tDumping ramdisk contents...\n");

	int fs_entry = 0;
	struct dirent *node = 0;
	while ( (node = readdir_fs(fs_root, fs_entry)) != 0)
	{
		dbg("\tFound entry %s\n", node->name);
        
		fs_node_t *fsnode = finddir_fs(fs_root, node->name);

		if ((fsnode->flags&0x7) == FS_DIRECTORY)
		{
			dbg("\t(Directory)\n");
		}
		else
		{
			char* buf = kmalloc(fsnode->length + 1);
			uint32_t sz = read_fs(fsnode, 0, fsnode->length, buf);
			buf[fsnode->length] = '\0';
			ASSERT(sz == fsnode->length);
			dbg("\t(File[Size: %d]) %s\n",sz, buf);
			kfree(buf);
		}

		fs_entry++;
	}
	#endif

	// Read the CMOS.
	struct cmos_rtc* time_data = kmalloc(sizeof(struct cmos_rtc));

	cmos_get_time(time_data);

	dbg("[%d/%d/%d, %d:%d:%d] Finished setup!\n", (int)time_data->day, (int)time_data->month, (int)time_data->year, (int)time_data->hour, (int)time_data->minute, (int)time_data->second);

	// Initialize the Scheduler.
	initialize_scheduler();

	// Loop this eternally. We are never coming back.
	while(1) {
	}

	// Halt early.
	// ASSERT("End of features, for now!" == 0);

	return 0xDEADBEEF;
}
