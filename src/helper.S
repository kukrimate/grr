/*
 * Assembly helpers
 */

.section .bss

/* Stack for the VMM */
vmm_stack_base:
.skip 8192
vmm_stack_top:

.section .text

/* Switch to the VMM stack */
.global vmm_stack_init
vmm_stack_init:

/* Move the return address into RAX */
movq (%rsp), %rax

/* Switch stacks */
movq $vmm_stack_top, %rsp

/* Jump back to the caller */
jmp *%rax
