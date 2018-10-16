; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_reload_kernel();'

[GLOBAL gdt_reload_kernel]     ; Allows the C code to link to this
[GLOBAL gdt_reload_user]     ; Allows the C code to link to this
[EXTERN gp]            ; Says that 'gp' is in another file
gdt_reload_kernel:
    lgdt [gp]        ; Load the GDT with our 'gp' which is a special pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:k_far   ; 0x08 is the offset to our kernel code segment: Far jump!
k_far:
    ret               ; Returns back to the C code (in kernel mode)!

gdt_reload_user:
    lgdt [gp]        ; Load the GDT with our 'gp' which is a special pointer
    mov ax, 0x20      ; 0x20 is the offset in the GDT to our user data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x018:u_far   ; 0x18 is the offset to our user code segment: Far jump!
u_far:
    ret               ; Returns back to the C code (in user mode)!