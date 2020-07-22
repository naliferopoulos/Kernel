#ifndef PROCESS_H
#define PROCESS_H

#include <kernel/regs.h>
#include <libk/types.h>

#define PROCESS_STACK_BASE 0x40000000	// 1GB
#define PROCESS_STACK_SIZE 0x10000	// 64KB

#define PROCESS_DATA_BASE  0x20000000	// 512MB
#define PROCESS_DATA_SIZE  0x400000	// 4MB

#define PROCESS_TEXT_BASE  0x10000000	// 256MB

// Process Statuses
#define PROC_READY	0x1
#define PROC_WAITING	0x2

typedef struct process {
	uint32_t pid;	
	char* name;

	struct regs registers;
	
	int alive;
	int status;
	
	uint32_t page_dir;
	
	int exit_status;

	// Since these values are page-aligned and contiguous for
	// loaded binaries, we can keep track of memory pools to 
	// return to the memory manager after we kill a process.
	uint32_t stack_start;
	uint32_t stack_end;
	uint32_t data_start;
	uint32_t data_end;
	uint32_t text_start;
	uint32_t text_end;

	struct process* parent;
	struct process* next;
} process_t;

void initialize_scheduler();
void context_switch(struct regs* r);
uint32_t create_kernel_process(void(*entry)(void), char* name);

#endif
