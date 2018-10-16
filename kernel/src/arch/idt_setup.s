; Loads the IDT defined in 'idtp' into the processor.
; This is declared in C as 'extern void idt_load();'

[GLOBAL idt_load]
[EXTERN idtp]
idt_load:
    lidt [idtp]
    ret
		