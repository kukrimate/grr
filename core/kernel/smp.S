/*
 * SMP initialization trampoline
 */

.section .text

/*
 * 16-bit SMP init trampoline, C code will memcpy this below 1MiB
 */
.global smp_init16
smp_init16:
.code16

/* Make sure DS matches CS */
movw %cs, %ax
movw %ax, %ds

/*
 * Load GDT
 * NOTE: the GDTR address is reltive to the start of the trampoline
 * as we are started in segmented realmode with IP=0
 */
lgdtw smp_fixup_gdtr - smp_init16

/* Enable protected mode */
movl %cr0, %eax
orl $1, %eax
movl %eax, %cr0

/* Jump into protected mode */
.global smp_fixup_init32
smp_fixup_init32:
ljmpl $0x08, $0xdeadbeef

/*
 * GDT register loading zone
 * NOTE: this must be inside the memcpy'd trampoline as we need to load
 * this while still in real mode
 */
.global smp_fixup_gdtr
smp_fixup_gdtr:
.word 0
.long 0

/* Mark the end of the trampoline so C code knows its size */
.global smp_init16_end
smp_init16_end:

/*
 * 32-bit SMP init
 */
.global smp_init32
smp_init32:
.code32
movl $0x10, %eax
movl %eax, %ds
movl %eax, %es
movl %eax, %ss
movl %eax, %fs
movl %eax, %gs

/* Enable PAE and PGE */
movl $0xa0, %eax
movl %eax, %cr4

/* Load the kernel's page table */
.global smp_fixup_pml4
smp_fixup_pml4:
movl $0xdeadbeef, %eax
movl %eax, %cr3

/* Set EFER.LME */
movl $0xc0000080, %ecx
rdmsr
orl $0x100, %eax
wrmsr

/* Enable paging */
movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0

/* Jump into long mode */
.global smp_fixup_init64
smp_fixup_init64:
ljmpl $0x18, $0xdeadbeef

/*
 * 64-bit SMP init
 */
.global smp_init64
smp_init64:
.code64
movl $0x20, %eax
movl %eax, %ds
movl %eax, %es
movl %eax, %ss
movl %eax, %fs
movl %eax, %gs

/* Setup 64-bit stack for this core */
.global smp_fixup_rsp
smp_fixup_rsp:
movq $0xdeadbeefdeadbeef, %rsp

/* Call into C code */
call acpi_smp_ap_entry

/* This should never be reached */
1: hlt
jmp 1b
