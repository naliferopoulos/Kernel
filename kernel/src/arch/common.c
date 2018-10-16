#include <arch/common.h>

void outb(u16int_t port, u8int_t value)
{
	__asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int_t inb(u16int_t port)
{
	u8int_t ret;
   	__asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   	return ret;
}

u16int_t inw(u16int_t port)
{
	u16int_t ret;
	__asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   	return ret;
}
