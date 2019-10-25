SRCS=	\
  	src/boot/boot.o \
	src/kernel/kernel.o \
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
	src/mem/vmm.o \
	src/mem/heap.o \
	src/libk/string.o \
	src/libk/stdio.o \
	src/libk/ctype.o \
	src/fs/fs.o \
	src/dev/initrd.o \
	src/task/spinlock.o \
	src/ds/bitmap.o \
	src/dev/serial.o \
	src/libk/dbg.o \
	src/ds/list.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -std=c11 -I ./inc -D DEBUG -g 
LDFLAGS=-T ./conf/link-kernel.ld -nostdlib -Map=bin/kernel.map
ASFLAGS= -f elf

LD=./toolchain/bin/i686-elf-ld
AS=nasm
CC=./toolchain/bin/i686-elf-gcc

all: 	$(SRCS) link ramdisk
	@printf "\n[+] Done!\n\n"
	@printf "\n\tWelcome to Kernel!\t\n\n"
	@printf "This is your first step towards insanity.\n"
	@printf "The code sucks, the bugs thrive, the features lack.\n"
	@printf "Considered yourself warned!\n"
	@printf "Have fun!\n"
	@printf "\n\t- Nick\n"

clean:
	@printf "\n[+] Cleaning...\n\n"
	@find . -name "*.o" -type f -delete
	@-rm -rf bin/kernel bin/initrd.img bin/kernel.log bin/kernel.map

run:
	@printf "\n[+] Running...\n\n"
	@-qemu-system-i386 -kernel ./bin/kernel -monitor stdio -name Kernel -initrd ./bin/initrd.img -m 512 -serial file:bin/kernel.log

debug:
	@printf "\n[+] Running in debug mode...\n\n"
	@-qemu-system-i386 -kernel ./bin/kernel -initrd ./bin/initrd.img -m 512 -s -S -serial file:bin/kernel.log &
	@-sleep 1
	@-gdb

link:
	@printf "\n[+] Creating Kernel Image...\n\n"
	@$(LD) $(LDFLAGS) -o ./bin/kernel $(SRCS)

utils:
	@printf "\n[+] Creating Utilities...\n\n"
	@-gcc utils/rdgen.c -o utils/rdgen

ramdisk:
	@printf "\n[+] Generating Ramdisk...\n\n"
	@-cd initrd && ../utils/rdgen ../bin/initrd.img * 

.c.o:
	@echo "[+] GCC $<"
	@$(CC) $(CFLAGS) -o $@ -c $< 

.s.o:
	@echo "[+] AS $<" 
	@$(AS) $(ASFLAGS) $<
