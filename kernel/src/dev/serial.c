#include <dev/serial.h>
#include <arch/common.h>

// COM1, for safety
#define PORT 0x3F8

void setup_serial() 
{
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int is_transmit_empty() 
{
   return inb(PORT + 5) & 0x20;
}
 
void serial_write_char(uint8_t c) 
{
   while (is_transmit_empty() == 0);
 
   outb(PORT, c);
}

void serial_write(uint8_t* s)
{
	int i = 0;
   	while (s[i])
   	{
   		serial_write_char(s[i++]);
   	}
}