SRCS=	\
  src/boot/boot.o \
	src/kernel/main_k.o \
	src/arch/common.o \
	src/dev/vga.o \
	src/libk/stdlib.o \
	src/arch/gdt.o \
	src/arch/gdt_setup.o \
	src/arch/idt.o \
	src/arch/idt_setup.o \
	src/arch/isr.o \
	src/arch/isr_setup.o \
	src/arch/irq.o \
	src/arch/irq_setup.o \
	src/dev/timer.o \
	src/dev/keyboard.o \
	src/mem/pmm.o \
	src/arch/paging_setup.o \
	src/mem/pde.o \
	src/mem/pte.o \
	src/mem/vmm.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -std=c11 -g -I ./inc
LDFLAGS=-T ./conf/link-kernel.ld
ASFLAGS= -f elf

LD=/opt/cross/bin/i386-elf-ld
AS=nasm
CC=/opt/cross/bin/i386-elf-gcc

all: $(SRCS) link

clean:
	-rm src/arch/*.o src/mem/*.o src/kernel/*.o src/libk/*.o src/dev/*.o src/boot/*.o  kernel

run:
	./run.sh

debug:
	./debug.sh

link:
	$(LD) $(LDFLAGS) -o kernel $(SRCS)

.s.o:
	$(AS) $(ASFLAGS) $<
