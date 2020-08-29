/*
 * Virtual machine monitor
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include "vmcb.h"
#include "uart.h"
#include "x86.h"

void vmm_pml4();
void vmm_pdp();

static
void
vmm_switchpg()
{
	uint64_t *pml4, *pdp, cur_phys;
	size_t i;

	pml4 = (uint64_t *) vmm_pml4;
	pdp = (uint64_t *) vmm_pdp;
	cur_phys = 0;

	for (i = 0; i < 512; ++i) {
		pdp[i] = cur_phys | 0x83;
		cur_phys += 0x40000000;
	}
	pml4[0] = (uint64_t) vmm_pdp | 3;
	asm volatile ("movq %0, %%cr3" :: "r" (vmm_pml4));
}

/*
 * Host save state
 */
extern
void host_save_state();

#define MSR_EFER	0xC0000080
# define EFER_SVME	(1 << 12)

#define MSR_VM_HSAVE_PA	0xC0010117

void
vmm_setup(struct vmcb *vmcb)
{
	/* Enable UART */
	uart_setup();

	/* Enable SVM */
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SVME);

	/* Set host save state address */
	wrmsr(MSR_VM_HSAVE_PA, (uint64_t) host_save_state);

	/* Setup VMCB */
	vmcb->guest_asid = 1;
	vmcb->vmrun = 1;
	vmcb->cpuid = 1;

	/* FIXME: the current segmentation setup has no GDT backing it,
		only hidden segmnet registers are set up, but Linux loads its
		own so we don't care for now */
	vmcb->es_selector = 0x18;
	vmcb->es_attrib = 0x0092;
	vmcb->cs_selector = 0x10;
	vmcb->cs_attrib = 0x029a;
	vmcb->ss_selector = 0x18;
	vmcb->ss_attrib = 0x0092;
	vmcb->ds_selector = 0x18;
	vmcb->ds_attrib = 0x0092;
	vmcb->fs_selector = 0x18;
	vmcb->fs_attrib = 0x0092;
	vmcb->gs_selector = 0x18;
	vmcb->gs_attrib = 0x0092;

	vmcb->cr0 = read_cr0();
	vmcb->cr3 = read_cr3();
	vmcb->cr4 = read_cr4();
	vmcb->efer = rdmsr(MSR_EFER) & ~EFER_SVME;

	vmm_switchpg();
}

#define VMEXIT_CPUID	0x72
#define VMEXIT_VMRUN	0x80

void
vmexit_handler(struct vmcb *vmcb, struct gprs *gprs)
{
	uint64_t rax, rbx, rcx, rdx;

	uart_print("#VMEXIT(0x%x)\n", vmcb->exitcode);
	switch (vmcb->exitcode) {
	case VMEXIT_CPUID:
		uart_print("CPUID EAX=%x\n", vmcb->rax);

		rax = vmcb->rax;
		asm volatile (
			"movq %0, %%rax\n"
			"cpuid\n"
			"movq %%rax, %0\n"
			"movq %%rbx, %1\n"
			"movq %%rcx, %2\n"
			"movq %%rdx, %3\n"
			: "=m" (rax), "=m" (rbx), "=m" (rcx), "=m" (rdx)
			:: "rax", "rbx", "rcx", "rdx");

		if (!vmcb->rax) { /* Fake CPUID to BootlegAMD */
			vmcb->rax = rax;
			gprs->rbx = 0x746f6f42;
			gprs->rcx = 0x0000444d;
			gprs->rdx = 0x4167656c;
		} else {
			vmcb->rax = rax;
			gprs->rbx = rbx;
			gprs->rcx = rcx;
			gprs->rdx = rdx;
		}

		vmcb->rip += 2;
		break;
	case VMEXIT_VMRUN:	/* The guest is not allowed this */
		break;
	}
}
