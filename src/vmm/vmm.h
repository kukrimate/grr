#ifndef VMM_H
#define VMM_H

void *
vmm_setup_core(void);

void
vmm_startup_bsp(void *vmcb, struct grr_handover *handover);

void
vmm_startup_ap(void *vmcb);

#endif
