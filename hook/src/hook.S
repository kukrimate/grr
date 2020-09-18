.section .text

/* Call MS ABI functions */
.macro mscall func
sub $0x20, %rsp
call \func
add $0x20, %rsp
.endm

/*
 * ExitBootServices handler:
 *  This hook will be installed the UEFI boot services table
 *  1. it calls the original ExitBootServices function
 *  2. it saves the callee saved registered in the handover blocks
 *  3. transfers control to the hypervisor in memory
 */
.global exit_boot_services_hook
exit_boot_services_hook:

/* Call firmware function */
movq exit_boot_services_orig, %rax
mscall *%rax

/* Load the address of the handover block */
movq handover, %rcx

/* Set the guest RIP to our return address */
popq %rax
movq %rax, 0x210(%rcx)	/* RIP */

/* Preserve the called saved registers */
movq %rbx, 0x230(%rcx)
movq %rsp, 0x238(%rcx)
movq %rbp, 0x240(%rcx)
movq %rsi, 0x248(%rcx)
movq %rdi, 0x250(%rcx)
movq %r12, 0x278(%rcx)
movq %r13, 0x280(%rcx)
movq %r14, 0x288(%rcx)
movq %r15, 0x290(%rcx)

/* Return EFI_SUCCESS to the caller in guest mode */
movq $0, 0x218(%rcx)

/*
 * Start the hypervisor
 * NOTE: that the hypervisor uses the SysV ABI, thus the first argument is RDI
 */
mov %rcx, %rdi
movq kernel_init, %rax
jmp *%rax
