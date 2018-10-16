#ifndef VGA_H
#define VGA_H

#include <arch/common.h>

// Write a single character out to the screen.
void monitor_put(char c);

// Clear the screen.
void monitor_clear();

// Output a null-terminated ASCII string to the monitor.
void monitor_write(char* c);

// Output a null-terminated ASCII string to the center of the line.
void monitor_write_center(char* c);

void monitor_write_hex(int i);

// Set the background color.
void monitor_set_bg_color(u8int_t color);

// Set the foreground color.
void monitor_set_fg_color(u8int_t color);

enum vga_color
{
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LGRAY = 7,
	DGRAY = 8,
	LBLUE = 9,
	LGREEN = 10,
	LCYAN = 11,
	LRED = 12,
	LMAGENTA = 13,
	YELLOW = 14,
	WHITE = 15
};

#endif // VGA_H
