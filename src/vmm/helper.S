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

/* VMM heap */
.global vmm_heap
vmm_heap:
.skip 0x100000
.global vmm_heap_end
vmm_heap_end:

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

/* Call MS ABI functions */
.macro mscall func
sub $0x20, %rsp
call \func
add $0x20, %rsp
.endm

.section .text

/* Start of the VMM */
.global vmm_startup
vmm_startup:
	/* Switch stacks */
	movq $vmm_stack_top, %rsp

	push %rcx	/* Entry point */
	push %rdx	/* Boot params */

	/* Load the VMM's GDT */
	lgdt vmm_gdtr
	pushq $0x08
	pushq $1f
	retfq
1:
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %ss
	movl %eax, %fs
	movl %eax, %gs

	/* Load the VMM's IDT */
	lidt vmm_idtr

	/* Setup VMCB */
	movq $vmcb, %rcx
	mscall vmm_setup

	/* Setup Host save state */
	movl $0xC0010117, %ecx	/* MSR_VM_HSAVE_PA */
	xorl %edx, %edx	/* FIXME: this only works if we're linked under 4G */
	movl $host_save_state, %eax
	wrmsr

	/* Guest parameters */
	pop %rsi
	pop %rax
	movq %rax, vmcb + 0x578
2:
	/* Start the VM */
	movq $vmcb, %rax
	vmload
	vmrun
	vmsave

	/* VMEXIT entry point */
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

	/* Do the actual work in C */
	movq $vmcb, %rcx
	movq %rsp, %rdx
	mscall vmexit_handler

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

	jmp 2b
