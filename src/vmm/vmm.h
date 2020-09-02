#ifndef VMM_H
#define VMM_H

void *
vmm_setup_core(void);

void
vmm_startup_bsp(void *vmcb, uint64_t kernel_entry, uint64_t boot_params);

void
vmm_startup_ap(void *vmcb);

#endif
