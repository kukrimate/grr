/*
 * exit_boot_services hook
 */
.global exit_boot_services_hook
exit_boot_services_hook:

/* Call firmware function */
movq exit_boot_services_orig, %rax
call *%rax

/* Save registers to handover block */
movq handover, %rcx
movq %rbx, 0x210(%rcx)
movq %rbp, 0x218(%rcx)
movq %rdi, 0x220(%rcx)
movq %rsi, 0x228(%rcx)
movq %rsp, 0x230(%rcx)
movq %r12, 0x238(%rcx)
movq %r13, 0x240(%rcx)
movq %r14, 0x248(%rcx)
movq %r15, 0x250(%rcx)

/* Start the hypervisor */
jmp kernel_init
