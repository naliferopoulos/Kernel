#include <task/process.h>
#include <mem/heap.h>
#include <arch/regs.h>
#include <arch/gdt.h>
#include <libk/dbg.h>
#include <libk/string.h>
#include <libk/stdlib.h>
#include <libk/stdio.h>

#define STACK_SZ 256

extern uint32_t kernel_pdbr;

int cur_pid = 0;
int scheduler = 0;

process_t* kernel_process = 0;
process_t* current_process = 0;

void a_proc()
{
	uint16_t* vid_mem = (uint16_t*)0xB8000;
	while(1){
		*vid_mem = 'A' | (((0 << 4) | (0xF & 0x0F)) << 8);
	}
}

void b_proc()
{
	uint16_t* vid_mem = (uint16_t*)0xB8000;
	while(1){
		*vid_mem = 'B' | (((0 << 4) | (0xF & 0x0F)) << 8);
	}
}

void kernel_proc()
{
	while(1){}
}

static void dump_proc(process_t* proc)
{
	dbg("\n");
	dbg("=== %s:%d ===\n", proc->name, proc->pid);
	
	dbg("Process Information\n");
	dbg("\tParent: '%s:%d', Next: '%s:%d'\n", proc->parent->name, proc->parent->pid, proc->next->name, proc->next->pid);
	dbg("\tScheduler: %s, Status:%s\n", proc->status == PROC_READY ? "Ready" : "Waiting", proc->alive == 1 ? "Alive" : "Dead");
	dbg("\tPage Directory: %x\n", proc->page_dir);

	dbg("Memory Map\n");
	dbg("\tStack: %x --> %x ", proc->stack_start, proc->stack_end);
	dbg("\tData: %x --> %x ", proc->data_start, proc->data_end);
	dbg("\tText: %x --> %x ", proc->text_start, proc->text_end);
	dbg("\n");

	dbg("Registers\n");
	dbg("\t[EAX]:%x [EBX]:%x [ECX]:%x [EDX]:%x [EDI]:%x [ESI]:%x\n", proc->registers.eax, proc->registers.ebx, proc->registers.ecx, proc->registers.edx, proc->registers.edi, proc->registers.esi);
	dbg("\t[ESP]:%x [USERESP]:%x [EBP]:%x [EIP]:%x\n", proc->registers.esp, proc->registers.useresp, proc->registers.ebp, proc->registers.eip);
	dbg("\t[GS]: %x [FS]:%x [ES]:%x [DS]:%x [SS]:%x [CS]:%x\n", proc->registers.gs, proc->registers.fs, proc->registers.es, proc->registers.ds, proc->registers.ss, proc->registers.cs);
	dbg("\n");
}

static void dump_processes()
{
	dbg("[SCHED]: Process List");
	process_t* p = current_process->next;
	
	while(p != current_process)
	{
		dump_proc(p);
		p = p->next;
	}

	dump_proc(current_process);
}

void initialize_scheduler()
{
	create_kernel_process(kernel_proc, "kernel");
	create_kernel_process(a_proc, "a_proc");
	create_kernel_process(b_proc, "b_proc");

	dump_processes();

	scheduler = 1;
}

static process_t* get_next_proc()
{
	ASSERT(current_process != 0);

	process_t* p = current_process->next;

	// Loop through the process ring to grab a ready process.
	while(p != current_process)
	{
		if(p->status == PROC_READY && p->alive == 1)
			return p;
		p = p->next;
	}

	return current_process;
}

void context_switch(struct regs* r)
{
	// If the scheduler is not initialized, return early.
	if (scheduler == 0)
		return;

	process_t* target = get_next_proc();

	// Skip over the switch if we are switching to the same task!
	if(target == current_process)
	{
		dbg("[SCHED] Skipping switch because an attempt to switch to the same process just happened!\n");
		return;
	}
	
	//dbg("[SCHED] Switching from task '%s:%d' to task '%s:%d'.\n", current_process->name, current_process->pid, target->name, target->pid);
	//dbg("[SCHED] Old Register State:\n");
	//dump_proc(current_process);
	//dbg("[SCHED] EBP before write:%x\n", r->ebp);

	// Save the current state.
	memcpy(&(current_process->registers), r, sizeof(struct regs));

	current_process = target;

	// Set current registers to the new processes's registers.
	memcpy(r, &(current_process->registers), sizeof(struct regs));

	//uint32_t addr = 0;
	//char* name = get_symbol(current_process->registers.eip, &addr);
	
	//uint32_t offset = current_process->registers.eip - addr;

	//dbg("[SCHED] Jumping to task '%s' at function <%s+%d>\n", current_process->name, name, offset);
	//dbg("[SCHED] Stack from %x to %x\n", current_process->stack_start, current_process->registers.useresp);
	//dump_proc(current_process);
	//dbg("[SCHED] EBP after write:%x\n", r->ebp);
	//dbg("\n");
}

uint32_t create_kernel_process(void(*entry)(void), char* name)
{
	// Allocate space for the new process.
	process_t* proc = kmalloc(sizeof(process_t));

	proc->name = kmalloc(sizeof(char) * strlen(name));

	strcpy(proc->name, name);
	
	ASSERT(proc > 0);

	// Nullify the registers 
	memset(&(proc->registers), 0, sizeof(struct regs));
	
	// Set the pid
	proc->pid = cur_pid++;

	// Set the state.
	proc->alive = 1;
	proc->exit_status = 255;
	proc->status = PROC_READY;

	// Allocate stack space.
	proc->stack_start = (uint32_t) kmalloc(sizeof(uint8_t) * STACK_SZ);
	memset((void*)proc->stack_start, 0xAA, (int)(sizeof(uint8_t) * STACK_SZ));
	proc->stack_end = proc->stack_start + sizeof(uint8_t) * STACK_SZ;

	// For kernel processes nothing else matters because nothing else is to be allocated. 
	proc->data_start = 0;
	proc->data_end = 0;

	proc->text_start = 0;
	proc->text_end = 0;

	// Set the page directory to the kernel page directory, since it is a kernel process.
	proc->page_dir = kernel_pdbr;

	proc->registers.gs = KERNEL_DATA_SEGMENT;
	proc->registers.fs = KERNEL_DATA_SEGMENT;
	proc->registers.es = KERNEL_DATA_SEGMENT;
	proc->registers.ds = KERNEL_DATA_SEGMENT;
	proc->registers.ss = KERNEL_DATA_SEGMENT;
	proc->registers.cs = KERNEL_CODE_SEGMENT;
	proc->registers.eflags = 0x0202;
	proc->registers.eip = (uint32_t) entry;
	//proc->registers.esp = proc->stack_end;
	proc->registers.useresp = proc->stack_end;
	proc->registers.ebp = proc->stack_end;
	
	if(current_process == 0)
	{
		// This is the first process. We have to create the ring.
		proc->next = proc;
		current_process = proc;

		// And also store this as a kernel process.
		kernel_process = proc;
	}
	else
	{
		// There already are processes. We have to add to the ring.
		process_t* proc1 = current_process;
		process_t* proc2 = current_process->next;

		proc1->next = proc;
		proc->next = proc2;
	}

	proc->parent = kernel_process;

	return proc->pid;
}
