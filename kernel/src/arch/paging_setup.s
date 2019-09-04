[GLOBAL paging_enable]
[GLOBAL paging_disable]
[GLOBAL pmm_get_PDBR]

paging_enable:
	mov	eax, cr0
	or eax, 0x80000000		;set bit 31
	mov	cr0, eax
	ret

paging_disable:
	mov	eax, cr0
	and eax, 0x7FFFFFFF		;clear bit 31
	mov	cr0, eax
	ret

pmm_get_PDBR:
	mov eax, cr3
	ret