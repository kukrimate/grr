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

/* VMM guest execute loop */
.global vmm_run_guest
vmm_run_guest:
	/* Start the VM */
	movq $vmcb, %rax
	vmrun

	/* We've got a #VMEXIT */
	push %r15
	push %r14
	push %r13
	push %r12
	push %r11
	push %r10
	push %r9
	push %r8
	push %rbp
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %rbx

	mov %rsp, %rcx
	call vmexit_handler

	pop %rbx
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rbp
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %r12
	pop %r13
	pop %r14
	pop %r15

	jmp vmm_run_guest

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
