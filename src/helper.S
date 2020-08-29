/*
 * Assembly helpers
 */

.section .bss

.align 4096

/* VMCB */
.global vmcb
vmcb:
.skip 4096

/* VMM host save state */
.global host_save_state
host_save_state:
.skip 4096

/* VMM PML4 */
.global vmm_pml4
vmm_pml4:
.skip 4096

/* VMM PDP */
.global vmm_pdp
vmm_pdp:
.skip 4096

/* Stack for the VMM */
vmm_stack_base:
.skip 8192
vmm_stack_top:

.section .data

/* GDT for the VMM */
.align 16
vmm_gdt:
.quad	0
.quad	0x00209a0000000000
.quad	0x0000920000000000

vmm_gdtr:
.word	vmm_gdtr - vmm_gdt
.quad	vmm_gdt

/* NULL idtr to get rid of the firmware code */
.align 16
vmm_idtr:
.word	0
.quad	0

.section .text

/* Switch to the VMM stack */
.global vmm_init
vmm_init:
	/* Load the VMM's GDT */
	lgdt vmm_gdtr
	pushq $0x08
	pushq $reload_cs
	retfq
reload_cs:
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %ss
	movl %eax, %fs
	movl %eax, %gs

	/* Load the VMM's IDT */
	lidt vmm_idtr

	/* Move the return address into RAX */
	movq (%rsp), %rax
	/* Switch stacks */
	movq $vmm_stack_top, %rsp
	/* Jump back to the caller */
	jmp *%rax

/*
 * VMM guest execute loop
 */
.global vmm_run_guest
vmm_run_guest:
	/* We never return, so might as well own the whole VMM stack */
	mov $vmm_stack_top, %rsp

	/* Move boot_params into place */
	mov %rcx, %rsi
1:
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

	movq %rsp, %rcx /* Pointer to the register save area */

	/* WARN: Allocate shadow space, otherwise gcc fucks up the stack */
	sub $0x20, %rsp
	call vmexit_handler
	add $0x20, %rsp

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

	jmp 1b
