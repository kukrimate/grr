/*
 * Assembly helpers
 */

/* Call MS ABI functions */
.macro mscall func
sub $0x20, %rsp
call \func
add $0x20, %rsp
.endm

.section .text

/* Start the BSP VMM */
.global vmm_startup_bsp
vmm_startup_bsp:
	push %rcx	/* VMCB */

	/* Setup VMCB */
	mscall vmm_setup

	jmp vmm_execute

ap_guest_entry:
	movl 0xfee00020, %eax
	shrl $24, %eax
1: 	cmpb sipi_core, %al
	jne 1b
	vmmcall
1:	jmp 1b

/* Start an AP VMM */
.global vmm_startup_ap
vmm_startup_ap:
	push %rcx	/* VMCB */

	/* Setup VMCB */
	movq $0, %rdx	/* No handover block on APs */
	mscall vmm_setup

	/* Make AP guest wait for SIPI from the BSP */
	movq 0(%rsp), %rcx
	movq $ap_guest_entry, 0x578(%rcx)

/* VMM execute loop */
vmm_execute:
	/* Release lock before starting guest */
	lock btrl $0, kernel_global_lock

	/* Start the VM */
	movq 0(%rsp), %rax
	vmload
	vmrun
	vmsave

	/* Acquire lock on VMEXIT */
1:	lock btsl $0, kernel_global_lock
	jc 1b

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
	movq 112(%rsp), %rcx
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

	jmp vmm_execute
