SRCS=boot.o main_k.o common.o vga.o kernel.o gdt.o gdt_setup.o idt.o idt_setup.o isr.o isr_setup.o irq.o irq_setup.o timer.o keyboard.o pmm.o paging_setup.o pde.o pte.o vmm.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -std=c11
LDFLAGS=-T link-kernel.ld
ASFLAGS= -f elf

LD=/opt/local/bin/i386-elf-ld
AS=nasm
CC=/opt/local/bin/i386-elf-gcc

all: $(SRCS) link

clean:
	-rm *.o kernel

run:
	./run.sh

link:
	$(LD) $(LDFLAGS) -o kernel $(SRCS)

.s.o:
	$(AS) $(ASFLAGS) $<
