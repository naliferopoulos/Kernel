#ifndef STDLIB_H
#define STDLIB_H

#define ASSERT(b) ((b) ? (void)0 : kpanicAssert(__FILE__, __LINE__, #b))

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */
};

int atoi(const char *str);
int strtoi(const char *str, char **endp, int base);

void itoa(long val, char *buf, int base);
void utoa(unsigned long val, char *buf, int base);

void kpanic(char* err, struct regs* r);
void* kmemset(void* b, int c, int len);
void abort();
void kstrace(int depth);
void kpanicAssert(char *file, int line, char *desc);

#endif
