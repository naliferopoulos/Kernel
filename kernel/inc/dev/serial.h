#ifndef SERIAL_H
#define SERIAL_H

#include <libk/types.h>

void setup_serial();
void serial_write_char(uint8_t c);
void serial_write(uint8_t* s);

#endif