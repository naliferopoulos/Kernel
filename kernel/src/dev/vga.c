#include <dev/vga.h>
#include <arch/common.h>

// The VGA framebuffer starts at 0xB8000.
uint16_t *video_memory = (uint16_t *)0xB8000;
// Stores the cursor position.
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

uint8_t bg_color = BLACK;
uint8_t fg_color = WHITE;

// Updates the hardware cursor.
static void move_cursor()
{
   // The screen is 80 characters wide...
   uint16_t cursorLocation = cursor_y * 80 + cursor_x;
   outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
   outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
   outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
   outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll()
{
   // Get a space character with the default colour attributes.
   //uint8_t attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint8_t attributeByte = (bg_color << 4) | (fg_color & 0x0F);
  	uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

   	// Row 25 is the end, this means we need to scroll up
   	if(cursor_y >= 25)
   	{
       // Move the current text chunk that makes up the screen
       // back in the buffer by a line
       int i;
       for (i = 0*80; i < 24*80; i++)
       {
           video_memory[i] = video_memory[i+80];
       }

       // The last line should now be blank. Do this by writing
       // 80 spaces to it.
       for (i = 24*80; i < 25*80; i++)
       {
           video_memory[i] = blank;
       }
       // The cursor should now be on the last line.
       cursor_y = 24;
   }
}

void monitor_set_bg_color(uint8_t color)
{
	bg_color = color;
}

void monitor_set_fg_color(uint8_t color)
{
	fg_color = color;
}

// Writes a single character out to the screen.
void monitor_put(char c)
{
   // The attribute byte is made up of two nibbles - the lower being the
   // foreground colour, and the upper the background colour.
   uint8_t  attributeByte = (bg_color << 4) | (fg_color & 0x0F);
   // The attribute byte is the top 8 bits of the word we have to send to the
   // VGA board.
   uint16_t attribute = attributeByte << 8;
   uint16_t *location;

   // Handle a backspace, by moving the cursor back one space
   if (c == 0x08 && cursor_x)
   {
      cursor_x--;
      location = video_memory + (cursor_y*80 + cursor_x);
      *location = ' ' | attribute;
   }

   // Handle a tab by increasing the cursor's X, but only to a point
   // where it is divisible by 8.
   else if (c == 0x09)
   {
      cursor_x = (cursor_x+8) & ~(8-1);
   }

   // Handle carriage return
   else if (c == '\r')
   {
       cursor_x = 0;
   }

   // Handle newline by moving cursor back to left and increasing the row
   else if (c == '\n')
   {
       cursor_x = 0;
       cursor_y++;
   }
   // Handle any other printable character.
   else if(c >= ' ')
   {
       location = video_memory + (cursor_y*80 + cursor_x);
       *location = c | attribute;
       cursor_x++;
   }

   // Check if we need to insert a new line because we have reached the end
   // of the screen.
   if (cursor_x >= 80)
   {
       cursor_x = 0;
       cursor_y ++;
   }

   // Scroll the screen if needed.
   scroll();
   // Move the hardware cursor.
   move_cursor();
}

// Clears the screen, by copying lots of spaces to the framebuffer.
void monitor_clear()
{
   // Make an attribute byte for the default colours
   uint8_t attributeByte = (bg_color << 4) | (fg_color & 0x0F);
   uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

   int i;
   for (i = 0; i < 80*25; i++)
   {
       video_memory[i] = blank;
   }

   // Move the hardware cursor back to the start.
   cursor_x = 0;
   cursor_y = 0;
   move_cursor();
}

// Outputs a null-terminated ASCII string to the monitor.
void monitor_write(char* c)
{
   int i = 0;
   while (c[i])
   {
       monitor_put(c[i++]);
   }
}

void monitor_write_center(char* c)
{
	int size = 0;
	while(c[size])
		size++;

	// 40 is half the screen width.
	int half;
	if(size % 2 != 0)
	{
		// We need to prepad!
		//monitor_put(' ');
		half = size / 2 -1;
		for(int i = 0; i < 40 - half - 1; i++)
			monitor_put(' ');
		monitor_write(c);
		for(int i = 0; i < 40 - half - 2; i++)
			monitor_put(' ');
	}
	else
	{
		half = size / 2;
		for(int i = 0; i < 40 - half; i++)
			monitor_put(' ');
		monitor_write(c);
		for(int i = 0; i < 40 - half; i++)
			monitor_put(' ');
	}
}

void monitor_write_hex(int i)
{
        monitor_write("0x");

        uint32_t counter = 8;
        uint8_t cur;

        while(counter-- > 0)
        {
                cur = (i & 0xf0000000) >> 28;
                i <<= 4;
                if (cur >= 0xA)
                  cur += 0x07;
                cur += 0x30;
                monitor_put(cur);
        }
}
