[GLOBAL irq0]
[GLOBAL irq1]
[GLOBAL irq2]
[GLOBAL irq3]
[GLOBAL irq4]
[GLOBAL irq5]
[GLOBAL irq6]
[GLOBAL irq7]
[GLOBAL irq8]
[GLOBAL irq9]
[GLOBAL irq10]
[GLOBAL irq11]
[GLOBAL irq12]
[GLOBAL irq13]
[GLOBAL irq14]
[GLOBAL irq15]

irq0:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 32
    jmp irq_common_stub

irq1:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 33
    jmp irq_common_stub

irq2:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 34
    jmp irq_common_stub

irq3:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 35
    jmp irq_common_stub

irq4:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 36
    jmp irq_common_stub

irq5:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 37
    jmp irq_common_stub

irq6:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 38
    jmp irq_common_stub

irq7:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 39
    jmp irq_common_stub

irq8:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 40
    jmp irq_common_stub

irq9:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 41
    jmp irq_common_stub

irq10:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 42
    jmp irq_common_stub

irq11:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 43
    jmp irq_common_stub

irq12:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 44
    jmp irq_common_stub

irq13:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 45
    jmp irq_common_stub

irq14:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 46
    jmp irq_common_stub

irq15:
    cli
    push byte 0    ; Note that these don't push an error code on the stack:
                   ; We need to push a dummy error code
    push byte 47
    jmp irq_common_stub

[EXTERN irq_handler]

irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    cld

    push esp
    call irq_handler
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iretd
