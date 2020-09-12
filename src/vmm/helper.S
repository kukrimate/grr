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

/* AP guest SIPI catcher */
.global ap_guest_entry
ap_guest_entry:
	movl 0xfee00020, %eax
	shrl $24, %eax
1: 	cmpb sipi_core, %al
	jne 1b
	vmmcall
1:	jmp 1b


/* VMM execute loop */
.global vmm_execute
vmm_execute:
	push %rcx	/* CPU context */

.execute_loop:
	/* Restore registers */
	movq (%rsp), %rax
	movq 0x1000(%rax), %rbx
	movq 0x1008(%rax), %rcx
	movq 0x1010(%rax), %rdx
	movq 0x1018(%rax), %rsi
	movq 0x1020(%rax), %rdi
	movq 0x1028(%rax), %rbp
	movq 0x1030(%rax), %r8
	movq 0x1038(%rax), %r9
	movq 0x1040(%rax), %r10
	movq 0x1048(%rax), %r11
	movq 0x1050(%rax), %r12
	movq 0x1058(%rax), %r13
	movq 0x1060(%rax), %r14
	movq 0x1068(%rax), %r15

	/* Release lock before starting guest */
	lock btrl $0, kernel_global_lock

	/* Start the VM */
	movq (%rsp), %rax
	vmload
	vmrun
	vmsave

	/* Acquire lock on VMEXIT */
1:	lock btsl $0, kernel_global_lock
	jc 1b

	/* Save registers */
	movq (%rsp), %rax
	movq %rbx, 0x1000(%rax)
	movq %rcx, 0x1008(%rax)
	movq %rdx, 0x1010(%rax)
	movq %rsi, 0x1018(%rax)
	movq %rdi, 0x1020(%rax)
	movq %rbp, 0x1028(%rax)
	movq %r8,  0x1030(%rax)
	movq %r9,  0x1038(%rax)
	movq %r10, 0x1040(%rax)
	movq %r11, 0x1048(%rax)
	movq %r12, 0x1050(%rax)
	movq %r13, 0x1058(%rax)
	movq %r14, 0x1060(%rax)
	movq %r15, 0x1068(%rax)

	/* Call high level #VMEXIT handler */
	movq %rax, %rcx
	mscall vmexit_handler

	jmp .execute_loop
