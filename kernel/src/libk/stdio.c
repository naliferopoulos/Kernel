#include <dev/vga.h>
#include <libk/stdio.h>
#include <libk/ctype.h>
#include <libk/limits.h>
#include <libk/stdbool.h>
#include <libk/stdarg.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <libk/stdlib.h>

int puts(const char* string) {
	return printf("%s\n", string);
}

int putchar(int ic) {
  char c = (char)ic;
	monitor_put(c);
  return ic;
}

static int print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

int printf(const char *restrict format, ...) {
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
      if (!print(format, amount))
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
      if (!print(&c, sizeof(c)))
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
      if (!print(str, len))
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
      if (!print(str, len))
        return -1;
      written += len;
    } else if (*format == 'x') {
      format++;
      print("0x", 2);
      char str[20];
      utoa(va_arg(parameters, uint64_t), str, 16);
      size_t len = strlen(str);
      if (maxrem < len) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (!print(str, len))
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
        if (!print(str, len))
          return -1;
        written += len;
      }
    } else if (*format == '0') {
      format++;
      if (*format == '2') {
        format++;
        if (*format == 'X') {
          format++;
          print("0x", 2);
          unsigned int i = va_arg(parameters, unsigned int);

          uint32_t counter = 8;
          uint8_t cur;

          while (counter-- > 0) {
            cur = (i & 0xf0000000) >> 28;
            i <<= 4;
            if (cur >= 0xA)
              cur += 0x07;
            cur += 0x30;
            putchar(cur);
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
      if (!print(format, len))
        return -1;
      written += len;
      format += len;
    }
  }

  va_end(parameters);
  return written;
}
