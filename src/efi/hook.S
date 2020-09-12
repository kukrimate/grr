/*
 * exit_boot_services hook
 */
.global exit_boot_services_hook
exit_boot_services_hook:

/* Call firmware function */
movq exit_boot_services_orig, %rax
call *%rax

/* We pass the handover block as an argument */
movq handover, %rcx

/* Set the guest RIP to our return address */
movq (%rsp), %rax
movq %rax, 0x210(%rcx)	/* RIP */
add $8, %rsp

/* We need to preserve the called saved registers */
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

/* Start the hypervisor */
jmp kernel_init
