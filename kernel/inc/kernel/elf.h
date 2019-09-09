#ifndef ELF_H
#define ELF_H

#include <libk/types.h>

typedef struct {
        uint32_t        name;
        uint32_t        addr;
        uint32_t        size;
        unsigned char   info;
        unsigned char   other;
        uint16_t        shndx;
} Elf32_Sym;

#endif
