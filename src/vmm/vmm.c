/*
 * Virtual machine monitor
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include <include/x86.h>
#include <kernel/acpi.h>
#include <kernel/alloc.h>
#include <kernel/kernel.h>
#include <kernel/uart.h>
#include "vmcb.h"

#define MSR_VM_CR	0xC0010114
# define VM_CR_R_INIT	(1 << 1)

#define MSR_VM_HSAVE_PA	0xC0010117

#define MSR_EFER	0xC0000080
# define EFER_SVME	(1 << 12)

struct vmcb *
vmm_setup_core(void)
{
	/* Enable SVM */
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SVME);

	/* Get exception on INIT */
	wrmsr(MSR_VM_CR, rdmsr(MSR_VM_CR) | VM_CR_R_INIT);

	/* Allocate and configure host save state */
	wrmsr(MSR_VM_HSAVE_PA, (uint64_t) alloc_pages(1, 0));

	/* Allocate VMCB */
	return alloc_pages(1, 0);
}

void
vmm_setup(struct vmcb *vmcb)
{
	/* Setup VMCB */
	vmcb->guest_asid = 1;
	vmcb->vmrun = 1;

	/* Intercept SX exception */
	vmcb->exception = (1 << 30);

	/* CPUID emulation */
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
	vmcb->efer = rdmsr(MSR_EFER);
}

static
uint64_t
read_guest_gpr(struct vmcb *vmcb, struct gprs *gprs, int idx)
{
	switch (idx) {
	case 0:
		return vmcb->rax;
	case 1:
		return gprs->rcx;
	case 2:
		return gprs->rdx;
	case 3:
		return gprs->rbx;
	case 4:
		return vmcb->rsp;
	case 5:
		return gprs->rbp;
	case 6:
		return gprs->rsi;
	case 7:
		return gprs->rdi;
	case 8:
		return gprs->r8;
	case 9:
		return gprs->r9;
	case 0xa:
		return gprs->r10;
	case 0xb:
		return gprs->r11;
	case 0xc:
		return gprs->r12;
	case 0xd:
		return gprs->r13;
	case 0xe:
		return gprs->r14;
	case 0xf:
		return gprs->r15;
	default: /* Bad idx */
		return 0;
	}
}

static
void
write_guest_gpr(struct vmcb *vmcb, struct gprs *gprs, int idx, uint64_t val)
{
	switch (idx) {
	case 0:
		vmcb->rax = val;
		break;
	case 1:
		gprs->rcx = val;
		break;
	case 2:
		gprs->rdx = val;
		break;
	case 3:
		gprs->rbx = val;
		break;
	case 4:
		vmcb->rsp = val;
		break;
	case 5:
		gprs->rbp = val;
		break;
	case 6:
		gprs->rsi = val;
		break;
	case 7:
		gprs->rdi = val;
		break;
	case 8:
		gprs->r8 = val;
		break;
	case 9:
		gprs->r9 = val;
		break;
	case 0xa:
		gprs->r10 = val;
		break;
	case 0xb:
		gprs->r11 = val;
		break;
	case 0xc:
		gprs->r12 = val;
		break;
	case 0xd:
		gprs->r13 = val;
		break;
	case 0xe:
		gprs->r14 = val;
		break;
	case 0xf:
		gprs->r15 = val;
		break;
	}
}

#define VMEXIT_EXP_SX	0x5e
#define VMEXIT_CPUID	0x72
#define VMEXIT_VMRUN	0x80

void
vmexit_handler(struct vmcb *vmcb, struct gprs *gprs)
{
	uint8_t *guest_rip;
	uint64_t rax, rbx, rcx, rdx;

	guest_rip = (uint8_t *) vmcb->rip;

	switch (vmcb->exitcode) {
	case VMEXIT_EXP_SX:
		uart_print("[%d] INIT caught\n", acpi_get_apic_id());
		break;
	case VMEXIT_CPUID:
		uart_print("[%d] CPUID EAX=%x\n",
			acpi_get_apic_id(), vmcb->rax);

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
	default:
		uart_print("[%d] Unknown #VMEXIT %d\n",
			acpi_get_apic_id(), vmcb->exitcode);
		for (;;)
			;
		break;
	}
}
