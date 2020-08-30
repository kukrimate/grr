/*
 * Kernel stack init
 */

/* Call MS ABI functions */
.macro mscall func
sub $0x20, %rsp
call \func
add $0x20, %rsp
.endm

.section .bss

/*
 * Stack for the bootstrap processor
 */
kernel_bsp_stack:
.skip 4096
kernel_bsp_stack_top:

.section .text

/*
 * Kernel entry point from EFI
 */
.global kernel_init
kernel_init:

/* Setup stack */
movq $kernel_bsp_stack_top, %rsp

/* Call C code */
mscall kernel_main

/* This should never be reached */
1: hlt
jmp 1b
