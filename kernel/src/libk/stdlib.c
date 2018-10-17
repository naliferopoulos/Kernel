#include <libk/stdlib.h>
#include <dev/vga.h>
#include <libk/stdio.h>

char* utoa(unsigned int val, char* s, int radix)
{
  static const char dig[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char* p, *q;

  q = s;
  do
  {
    *q++ = dig[val % radix];
    val /= radix;
  } while (val);

  *q = '\0';

  // Reverse the string (but leave the \0)
  p = s;
  q--;

  while (p < q)
  {
    char c = *p;
    *p++ = *q;
    *q-- = c;
  }

  return s;
}

char* itoa(int val, char* s, int radix)
{
  if (val < 0)
  {
    *s = '-';
    utoa(-val, s + 1, radix);
  }
  else
  {
    utoa(val, s, radix);
  }
  return s;
}

void abort()
{
   	while(1)
   	{
   	}
}

void kstrace(int depth)
{
    // Stack contains:
    //  Second function argument
    //  First function argument (depth)
    //  Return address in calling function
    //  EBP of calling function (pointed to by current EBP)
    int * ebp = (int *)(&depth - 2);
    int * ra = (int *)(&depth - 1);

    printf("Stack trace: (Return to <");
    monitor_write_hex(ra);
    printf(">)\n");

    for(int frame = 0; frame < depth; frame++)
    {
        if(ebp[1] == 0)
          break;
        int eip = ebp[1];

        // Unwind to previous stack frame
        ebp = (int *)(ebp[0]);
        //unsigned int * arguments = &ebp[2];

        printf("    <");
        monitor_write_hex(eip);
        printf(">\n");
    }
}

void kpanic(char* err, struct regs* r)
{
	monitor_set_fg_color(YELLOW);
	monitor_set_bg_color(RED);
	monitor_clear();

	for(int i = 0; i < 80; i++)
		putchar('-');
	monitor_write_center("KERNEL PANIC");
	for(int i = 0; i < 80; i++)
		monitor_put('-');
	monitor_write_center(err);
	monitor_write("\n");

	if(r != 0)
	{
		monitor_write("EAX: ");
		monitor_write_hex(r->eax);
		monitor_write(" ");
		monitor_write("EBX: ");
		monitor_write_hex(r->ebx);
		monitor_write(" ");
		monitor_write("ECX: ");
		monitor_write_hex(r->ecx);
		monitor_write(" ");
		monitor_write("EDX: ");
		monitor_write_hex(r->edx);
		monitor_write("\n");
		monitor_write("EIP: ");
		monitor_write_hex(r->eip);
		monitor_write(" ");
		monitor_write("EBP: ");
		monitor_write_hex(r->ebp);
		monitor_write(" ");
		monitor_write("ESP: ");
		monitor_write_hex(r->esp);
		monitor_write(" ");
		monitor_write("ESI: ");
		monitor_write_hex(r->esi);
		monitor_write("\n");
		monitor_write("EDI: ");
		monitor_write_hex(r->edi);
		monitor_write(" ");
		monitor_write("DS:  ");
		monitor_write_hex(r->ds);
		monitor_write(" ");
		monitor_write("ES:  ");
		monitor_write_hex(r->es);
		monitor_write(" ");
		monitor_write("GS:  ");
		monitor_write_hex(r->gs);
		monitor_write("\n");
		monitor_write("FS:  ");
		monitor_write_hex(r->fs);
		monitor_write(" ");
		monitor_write("CS:  ");
		monitor_write_hex(r->cs);
		monitor_write(" ");
		monitor_write("SS:  ");
		monitor_write_hex(r->ss);
		monitor_write(" ");
		monitor_write("ERR: ");
		monitor_write_hex(r->err_code);
		monitor_write("\n");
		monitor_write("EFLAGS: ");
		monitor_write_hex(r->eflags);
		monitor_write("\n");
		monitor_write("USRESP: ");
		monitor_write_hex(r->useresp);
		monitor_write("\n");
		monitor_write("INTNO:  ");
		monitor_write_hex(r->int_no);
		monitor_write("\n");
	}

	monitor_write("\n");
	monitor_write("\n");
	monitor_write("\n");
	monitor_write("\n");
	monitor_write("\n");
	/*
	monitor_write_center("Dedicated to G,");
	monitor_write_center("who helps me fix all the errors in my life.");
	*/
	monitor_write_center("We crashed and that is sad,");
	monitor_write_center("but if you send a screenshot our way we might be able to fix that.");
	monitor_write_center("Thank you for using Kernel! :)");

  abort();
}

void* kmemset(void *b, int c, int len)
{
  int i;
  unsigned char *p = b;
  i = 0;
  while(len > 0)
    {
      *p = c;
      p++;
      len--;
    }
  return(b);
}

void kpanicAssert(char *file, int line, char *desc)
{
  char string[5];
  itoa(line, string, 10);

  printf("Assertion Failed (%s) at %s:%s\n", desc, file, string);

  abort();

}
