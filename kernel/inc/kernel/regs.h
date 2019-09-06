#ifndef REGS_H
#define REGS_H

#include <libk/types.h>

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    uint32_t gs, fs, es, ds;      						/* pushed the segs last */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  	/* pushed by 'pusha' */
    uint32_t int_no, err_code;    						/* our 'push byte #' and ecodes do this */
    uint32_t eip, cs, eflags, useresp, ss;   			/* pushed by the processor automatically */
};

#endif