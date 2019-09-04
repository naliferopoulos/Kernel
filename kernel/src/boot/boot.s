;
; boot.s -- Kernel start location. Also defines multiboot header.
;

MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)


[BITS 32]                       ; All instructions should be 32-bit.

[GLOBAL mboot]                  ; Make 'mboot' accessible from C.
[EXTERN code]                   ; Start of the '.text' section.
[EXTERN bss]                    ; Start of the .bss section.
[EXTERN end]                    ; End of the last loadable section.

mboot:
  dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
  dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
  dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct

  dd  mboot                     ; Location of this descriptor
  dd  code                      ; Start of kernel '.text' (code) section.
  dd  bss                       ; End of kernel '.data' section.
  dd  end                       ; End of kernel.
  dd  start                     ; Kernel entry point (initial EIP).

[GLOBAL start]                  ; Kernel entry point.
[EXTERN k_main]                 ; Kernel k_main().


; page directory table
%define   PAGE_DIR      0x9C000

; 0th page table. Address must be 4KB aligned
%define   PAGE_TABLE_0    0x9D000

; 768th page table. Address must be 4KB aligned
%define   PAGE_TABLE_768    0x9E000

; each page table has 1024 entries
%define   PAGE_TABLE_ENTRIES  1024

; attributes (page is present;page is writable; supervisor mode)
%define   PRIV        3

; The virtual base address of the kernel
%define KERN_VIRT_BASE 0xc0000000

;****************************************
; Enable Paging
;****************************************

EnablePaging:
  pusha                   ; save stack frame

  ;------------------------------------------
  ; idenitity map 1st page table (4MB)
  ;------------------------------------------

  mov   eax, PAGE_TABLE_0         ; first page table
  mov   ebx, 0x0 | PRIV           ; starting physical address of page
  mov   ecx, PAGE_TABLE_ENTRIES       ; for every page in table...
.loop:
  mov   dword [eax], ebx          ; write the entry
  add   eax, 4                ; go to next page entry in table (Each entry is 4 bytes)
  add   ebx, 4096             ; go to next page address (Each page is 4Kb)
  loop  .loop               ; go to next entry

  ;------------------------------------------
  ; set up the entries in the directory table
  ;------------------------------------------

  mov   eax, PAGE_TABLE_0 | PRIV      ; 1st table is directory entry 0
  mov   dword [PAGE_DIR], eax

  mov   eax, PAGE_TABLE_768 | PRIV      ; 768th entry in directory table
  mov   dword [PAGE_DIR+(768*4)], eax

  ;------------------------------------------
  ; install directory table
  ;------------------------------------------

  mov   eax, PAGE_DIR
  mov   cr3, eax

  ;------------------------------------------
  ; enable paging
  ;------------------------------------------

  mov   eax, cr0
  or    eax, 0x80000000
  mov   cr0, eax

  ;------------------------------------------
  ; map the 768th table to physical addr 1MB
  ; the 768th table starts the 3gb virtual address
  ;------------------------------------------

  mov   eax, PAGE_TABLE_768       ; first page table
  mov   ebx, 0x100000 | PRIV      ; starting physical address of page
  mov   ecx, PAGE_TABLE_ENTRIES     ; for every page in table...
.loop2:
  mov   dword [eax], ebx        ; write the entry
  add   eax, 4              ; go to next page entry in table (Each entry is 4 bytes)
  add   ebx, 4096           ; go to next page address (Each page is 4Kb)
  loop  .loop2              ; go to next entry

  popa
  ret

global start
start:
  cli                         ; Disable interrupts.
  mov esp, stack 	      ; Set up the stack.
  call EnablePaging           ; Enable paging and remap kernel to 3GB.
  lea ecx, [higher_half]      ; Jump to higher half kernel.
  jmp ecx
higher_half:
  xor ebp, ebp                ; Prepare NULL stack frame so traces know where to stop
  push ebp 
  push eax		      ; Pass in the multiboot magic number.
  ;add ebx, KERN_VIRT_BASE     ; Pass in the virtual for the multiboot info. 
  push ebx                    ; Load multiboot header location
  call k_main                 ; Call kernel k_main().
  jmp $                       ; Enter an infinite loop, to stop the processor
                              ; executing whatever rubbish is in the memory
                              ; after our kernel!

section .bss
resb 8192
stack:
