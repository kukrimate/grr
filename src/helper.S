/*
 * Assembly helpers
 */

.section .bss

/* Stack for the VMM */
vmm_stack_base:
.skip 8192
vmm_stack_top:

.section .data

/* NULL idtr to get rid of the firmware code */
null_idtr:
.word	0
.quad	0

.section .text

/* Switch to the VMM stack */
.global vmm_stack_init
vmm_stack_init:

/* Kill the firmware's IDT */
lidt null_idtr

/* Move the return address into RAX */
movq (%rsp), %rax

/* Switch stacks */
movq $vmm_stack_top, %rsp

/* Jump back to the caller */
jmp *%rax

/* Self modifying code ;) */
.global guest_entry
guest_entry:
.global ge_kernel_addr
ge_kernel_addr:
movq $0xdeadbeefdeadbeef, %rax /* kernel_addr */
.global ge_boot_params
ge_boot_params:
movq $0xdeadbeefdeadbeef, %rsi /* boot_params */
jmp *%rax
