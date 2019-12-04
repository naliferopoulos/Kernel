# Kernel ![alt Kernel CI](https://github.com/naliferopoulos/Kernel/workflows/Kernel%20CI/badge.svg) 
x86 Homebrew'd Microkernel

![alt Kernel Panic](https://github.com/naliferopoulos/Kernel/blob/master/kernel_panic.png/)

## Why 'Kernel'
Starting a project like this I never imagined it would end up on Github, let alone being actually used. It is merely a hobby project that I started for research purposes. Given that, and my *fairly* limited imagination, **Kernel** was born.

## Cool, what does it do?
Kernel is only targetting x86 (i386+) computers and as of now it supports Multiboot (GRUB will *probably* load it), Paging, Physical and Virtual Memory Managers, Interrupts, Exception Handling, PIC, a simple Keyboard driver and a Text Mode VGA driver. Application loading will come in the next updates.

## Nice! How do I compile?
Compiling and correctly linking the kernel requires setting up a cross-compiler and toolchain (GCC and Binutils) for your system. Kernel will **not** compile correctly under your platform specific toolchain **even** if you are running x86. You can try passing flags to GCC to get it to compile in freestanding mode without linking to your OS's libraries but you are on your own. Consider yourself warned!

In order to build a cross-compiler, run the script provided in the utils to set up a toolchain automatically, by typing *make toolchain*.

After you have set up your cross-toolchain ~~edit the Makefile so that it points to your cross-binaries,~~ type *make* and voila!

## I have my binaries, now what?
You can test it under QEMU, or you can set up a partition with GRUB (or even LILO, SystemCommander and so on) to boot it. Type *make run*.

## It crashed! :(
This will probably happen more often than not. Send me a screenshot of the panic screen and I might be able to fix it!

Wait, are you still reading? If so, you may also be crazy enough to try and debug Kernel in order to identify and/or patch the problem. *make debug* right away, to set up remote debugging with GDB & QEMU, then type *file kernel* to pass symbols to GDB and *target remote locahost:1234* to make it attach to QEMU. Then you can debug Kernel like any other binary (with a little bit of bonus SEGFAULT). Oh, also, you can use QEMU's monitor which is attached to stdin/stdout when *make run* to identify bugs on low-level, arch specific components.

Go nuts :)

### *Kernel* is made with <3 and GCC. Special thanks to the patient and helpful community of the OSDev Forums, James Molloy, BrokenThorn Entertainment and Bran M. for their excellent OS Development material.
