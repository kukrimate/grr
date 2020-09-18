/*
 * Kernel stack init
 */

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
leaq kernel_bsp_stack_top(%rip), %rsp

/* Call C code */
call kernel_main

/* This should never be reached */
1: hlt
jmp 1b
