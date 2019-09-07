#include <dev/serial.h>
#include <libk/dbg.h>
#include <libk/stdio.h>
#include <libk/ctype.h>
#include <libk/limits.h>
#include <libk/stdbool.h>
#include <libk/stdarg.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/stdlib.h>

void abort()
{
    while(1)
    {
    }
}

static int dbg_putchar(int ic) {
  char c = (char)ic;
  serial_write_char(c);
  return ic;
}

static int dbg_print(const char* data, size_t length) {
  const unsigned char* bytes = (const unsigned char*) data;
  for (size_t i = 0; i < length; i++)
    if(dbg_putchar(bytes[i]) == EOF)
      return false;
    return true;
}

int dbgf(const char *restrict format, ...) {
  va_list parameters;
  va_start(parameters, format);

  int written = 0;

  while (*format != '\0') {
    size_t maxrem = INT_MAX - written;

    if (format[0] != '%' || format[1] == '%') {
      if (format[0] == '%')
        format++;
      size_t amount = 1;
      while (format[amount] && format[amount] != '%')
        amount++;
      if (maxrem < amount) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(format, amount))
        return -1;
      format += amount;
      written += amount;
      continue;
    }

    const char *format_begun_at = format++;

    if (*format == 'c') {
      format++;
      char c = (char)va_arg(parameters, int /* char promotes to int */);
      if (!maxrem) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(&c, sizeof(c)))
        return -1;
      written++;
    } else if (*format == 'd') {
      format++;
      char str[10];
      itoa(va_arg(parameters, int), str, 10);
      size_t len = strlen(str);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(str, len))
        return -1;
      written += len;
    } else if (*format == 's') {
      format++;
      const char *str = va_arg(parameters, const char *);
      size_t len = strlen(str);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(str, len))
        return -1;
      written += len;
    } else if (*format == 'x') {
      format++;
      dbg_print("0x", 2);
      char str[20];
      utoa(va_arg(parameters, uint64_t), str, 16);
      size_t len = strlen(str);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(str, len))
        return -1;
      written += len;
    } else if (*format == 'l') {
      format++;
      if (*format == 'd') {
        format++;
        char str[20];
        utoa(va_arg(parameters, long int), str, 10);
        size_t len = strlen(str);
        if (maxrem < len) {
          // TODO: Set errno to EOVERFLOW.
          return -1;
        }
        if (!dbg_print(str, len))
          return -1;
        written += len;
      }
    } else if (*format == '0') {
      format++;
      if (*format == '2') {
        format++;
        if (*format == 'X') {
          format++;
          dbg_print("0x", 2);
          unsigned int i = va_arg(parameters, unsigned int);

          uint32_t counter = 8;
          uint8_t cur;

          while (counter-- > 0) {
            cur = (i & 0xf0000000) >> 28;
            i <<= 4;
            if (cur >= 0xA)
              cur += 0x07;
            cur += 0x30;
            dbg_putchar(cur);
          }
        }
      }
    } else {
      format = format_begun_at;
      size_t len = strlen(format);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!dbg_print(format, len))
        return -1;
      written += len;
      format += len;
    }
  }

  va_end(parameters);
  return written;
}

void kstrace(uint32_t dummy)
{
    // Stack contains:
    //  Second function argument
    //  First function argument (depth)
    //  Return address in calling function
    //  EBP of calling function (pointed to by current EBP)
    
    uint32_t frame = 0;    
    int * ebp = (int *)(&dummy - 2);

    dbg("Stack trace:\n");

    while(1)
    {
        uint32_t eip = ebp[1];
        if(ebp == 0)
          break;

        // Unwind to previous stack frame
        ebp = (uint32_t *)(ebp[0]);
        uint32_t * arguments = &ebp[2];

        dbg("\t<%x>\n", eip);

        ++frame;
    }
}

void kpanicAssert(char *file, int line, char *desc)
{
  dbg("Assertion Failed (%s) at %s:%d\n", desc, file, line);
  kpanic(desc, NULL);
}

void kpanic(char* err, struct regs* r)
{
  dbg("\n\n");

  dbg("===============================\n");
  dbg("KERNEL PANIC <Guru meditation!>\n");
  dbg("===============================\n");

  dbg("\n\n");

  if(r != 0)
  {
    dbg("[EAX] %x\n", r->eax);
    dbg("[EBX] %x\n", r->ebx);
    dbg("[ECX] %x\n", r->ecx);
    dbg("[EDX] %x\n", r->edx);

    dbg("\n");

    dbg("[EIP] %x\n", r->eip);
    dbg("[EBP] %x\n", r->ebp);
    dbg("[ESP] %x\n", r->esp);

    dbg("\n");

    dbg("[ESI] %x\n", r->esi);
    dbg("[EDI] %x\n", r->edi);

    dbg("\n");

    dbg("[DS] %x\n", r->ds);
    dbg("[ES] %x\n", r->es);
    dbg("[GS] %x\n", r->gs);
    dbg("[FS] %x\n", r->fs);
    dbg("[CS] %x\n", r->cs);
    dbg("[SS] %x\n", r->ss);

    dbg("\n");

    dbg("[ERR] %x\n", r->err_code);    
    dbg("[EFLAGS] %x\n", r->eflags);
    dbg("[USRESP] %x\n", r->useresp);

    dbg("\n");

    dbg("[INTNO] %x\n", r->int_no);

    dbg("\n");
  }

  /*
  monitor_write_center("Dedicated to G,");
  monitor_write_center("who helps me fix all the errors in my life.");
  */
 
  kstrace(0);

  dbg("Enjoy some ascii art found here:\n");
  dbg("https://www.asciiart.eu/electronics/robots\n");
  dbg("                                       |       \n");
  dbg("                                       |       \n");
  dbg("                                       |       \n");
  dbg("                                       |       \n");
  dbg(" _______                   ________    |       \n");
  dbg("|ooooooo|      ____       | __  __ |   |       \n");
  dbg("|[]+++[]|     [____]      |/  \\/  \\|   |       \n");
  dbg("|+ ___ +|     ]()()[      |\\__/\\__/|   |       \n");
  dbg("|:|   |:|   ___\\__/___    |[][][][]|   |       \n");
  dbg("|:|___|:|  |__|    |__|   |++++++++|   |       \n");
  dbg("|[]===[]|   |_|_/\\_|_|    | ______ |   |       \n");
  dbg("||||||||| _ | | __ | | __ ||______|| __|       \n");
  dbg("|_______|   |_|[::]|_|    |________|   \\       \n");
  dbg("            \\_|_||_|_/                  \\      \n");
  dbg("              |_||_|                     \\     \n");
  dbg("             _|_||_|_                     \\    \n");
  dbg("    ____    |___||___|                     \\   \n");
  dbg("   /  __\\          ____                     \\  \n");
  dbg("   \\( oo          (___ \\                     \\ \n");
  dbg("   _\\_o/           oo~)/                       \n");
  dbg("  / \\|/ \\         _\\-_/_                       \n");
  dbg(" / / __\\ \\___    / \\|/  \\                      \n");
  dbg(" \\ \\|   |__/_)  / / .- \\ \\                     \n");
  dbg("  \\/_)  |       \\ \\ .  /_/                     \n");
  dbg("   ||___|        \\/___(_/                      \n");
  dbg("   | | |          | |  |                       \n");
  dbg("   | | |          | |  |                       \n");
  dbg("   |_|_|          |_|__|                       \n");
  dbg("   [__)_)        (_(___]                       \n");

  abort();
}
