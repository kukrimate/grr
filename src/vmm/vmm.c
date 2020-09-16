/*
 * Virtual machine monitor
 */

#include <stddef.h>
#include <stdint.h>
#include <include/x86.h>
#include <include/handover.h>
#include <kernel/acpi.h>
#include <kernel/alloc.h>
#include <kernel/kernel.h>
#include <kernel/string.h>
#include <kernel/uart.h>
#include <vmm/vmm.h>

#define MSR_VM_CR	0xC0010114
# define VM_CR_R_INIT	(1 << 1)

#define MSR_VM_HSAVE_PA	0xC0010117

#define MSR_EFER	0xC0000080
# define EFER_SVME	(1 << 12)

static
struct vmm_cpu *
vmm_setup_cpu(void)
{
	/* Enable SVM */
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SVME);
	/* Allocate and configure host save state */
	wrmsr(MSR_VM_HSAVE_PA, (uint64_t) alloc_pages(1, 0));

	/* Allocate CPU context */
	return alloc_pages(PAGE_COUNT(sizeof(struct vmm_cpu)), 0);
}

static
void
vmm_setup_npt(struct vmm_cpu *ctx)
{
	uint64_t cur_phys, *pml4, *pdp, *pd;
	size_t pdp_idx, pd_idx;

	cur_phys = 0;
	pml4 = alloc_pages(1, NULL);
	pdp = alloc_pages(1, NULL);

	for (pdp_idx = 0; pdp_idx < 512; ++pdp_idx) {
		pd = alloc_pages(1, NULL);
		for (pd_idx = 0; pd_idx < 512; ++pd_idx) {
			if (cur_phys == (uint64_t) lapic_addr) {
				/* FIXME: this leaves a
					memory hole after the LAPIC */
				ctx->lapic_pt = alloc_pages(1, NULL);
				ctx->lapic_pt[0] = (uint64_t) cur_phys | 5;
				pd[pd_idx] = (uint64_t) ctx->lapic_pt | 7;
			} else {
				pd[pd_idx] = cur_phys | 0x87;
			}
			cur_phys += 0x200000;
		}
		pdp[pdp_idx] = (uint64_t) pd | 7;
	}
	pml4[0] = (uint64_t) pdp | 7;

	ctx->vmcb.np_en = 1;
	ctx->vmcb.n_cr3 = (uint64_t) pml4;

	/* LAPIC emulator page */
	ctx->lapic_emu = alloc_pages(1, NULL);
}


struct vmm_cpu *
vmm_setup_bsp(struct grr_handover *handover)
{
	struct vmm_cpu *ctx;

	ctx = vmm_setup_cpu();

	/* Setup VMCB */
	ctx->vmcb.guest_asid = 1;
	ctx->vmcb.vmrun = 1;
	ctx->vmcb.vmmcall = 1;

	/* Enable nested paging */
	vmm_setup_npt(ctx);
	/* Catch #DB for MMIO emulation */
	ctx->vmcb.exception = (1 << 1);

	/* CPUID emulation */
	ctx->vmcb.cpuid = 1;

	/* FIXME: the current segmentation setup has no GDT backing it,
		only hidden segmnet registers are set up, but Linux loads its
		own so we don't care for now */
	ctx->vmcb.es_selector = 0x18;
	ctx->vmcb.es_attrib = 0x0092;
	ctx->vmcb.cs_selector = 0x10;
	ctx->vmcb.cs_attrib = 0x029a;
	ctx->vmcb.ss_selector = 0x18;
	ctx->vmcb.ss_attrib = 0x0092;
	ctx->vmcb.ds_selector = 0x18;
	ctx->vmcb.ds_attrib = 0x0092;
	ctx->vmcb.fs_selector = 0x18;
	ctx->vmcb.fs_attrib = 0x0092;
	ctx->vmcb.gs_selector = 0x18;
	ctx->vmcb.gs_attrib = 0x0092;

	/* BSP initial state comes from the handover block */
	ctx->vmcb.rip = handover->rip;
	ctx->vmcb.rsp = handover->rsp;
	ctx->vmcb.rax = handover->rax;
	ctx->gprs.rbx = handover->rbx;
	ctx->gprs.rcx = handover->rcx;
	ctx->gprs.rdx = handover->rdx;
	ctx->gprs.rsi = handover->rsi;
	ctx->gprs.rdi = handover->rdi;
	ctx->gprs.rbp = handover->rbp;
	ctx->gprs.r8  = handover->r8;
	ctx->gprs.r9  = handover->r9;
	ctx->gprs.r10 = handover->r10;
	ctx->gprs.r11 = handover->r11;
	ctx->gprs.r12 = handover->r12;
	ctx->gprs.r13 = handover->r13;
	ctx->gprs.r14 = handover->r14;
	ctx->gprs.r15 = handover->r15;

	ctx->vmcb.cr0 = read_cr0();
	ctx->vmcb.cr3 = read_cr3();
	ctx->vmcb.cr4 = read_cr4();
	ctx->vmcb.efer = rdmsr(MSR_EFER);

	return ctx;
}

void ap_guest_entry();

struct vmm_cpu *
vmm_setup_ap(void)
{
	struct vmm_cpu *ctx;

	ctx = vmm_setup_cpu();

	/* Setup VMCB */
	ctx->vmcb.guest_asid = 1;
	ctx->vmcb.vmrun = 1;
	ctx->vmcb.vmmcall = 1;

	/* CPUID emulation */
	ctx->vmcb.cpuid = 1;

	/* FIXME: the current segmentation setup has no GDT backing it,
		only hidden segmnet registers are set up, but Linux loads its
		own so we don't care for now */
	ctx->vmcb.es_selector = 0x18;
	ctx->vmcb.es_attrib = 0x0092;
	ctx->vmcb.cs_selector = 0x10;
	ctx->vmcb.cs_attrib = 0x029a;
	ctx->vmcb.ss_selector = 0x18;
	ctx->vmcb.ss_attrib = 0x0092;
	ctx->vmcb.ds_selector = 0x18;
	ctx->vmcb.ds_attrib = 0x0092;
	ctx->vmcb.fs_selector = 0x18;
	ctx->vmcb.fs_attrib = 0x0092;
	ctx->vmcb.gs_selector = 0x18;
	ctx->vmcb.gs_attrib = 0x0092;

	/* Setup intial guest state */
	ctx->vmcb.rip = (uint64_t) ap_guest_entry;

	ctx->vmcb.cr0 = read_cr0();
	ctx->vmcb.cr3 = read_cr3();
	ctx->vmcb.cr4 = read_cr4();
	ctx->vmcb.efer = rdmsr(MSR_EFER);

	return ctx;
}

#if 0
#define PT_ADDR(x)	(x & 0xffffffffff000)
#define PT_ADDR_HUGE(x)	(x & 0xfffffffffe000)

static
void *
guest_pgwalk(uint64_t *pml4, uint64_t virt_addr)
{
	uint64_t pml4_idx, pdp_idx, pd_idx, pt_idx;
	uint64_t *tmp_ptr;

	pml4_idx = virt_addr >> 39 & 0x1ff;
	pdp_idx  = virt_addr >> 30 & 0x1ff;
	pd_idx   = virt_addr >> 21 & 0x1ff;
	pt_idx   = virt_addr >> 12 & 0x1ff;

	/* Page map level 4 */
	if (!(pml4[pml4_idx] & 1))
		return NULL;
	tmp_ptr = (void *) PT_ADDR(pml4[pml4_idx]);

	/* Page directory pointer */
	if (!(tmp_ptr[pdp_idx] & 1))
		return NULL;
	else if (tmp_ptr[pdp_idx] & 0x80) /* 1 GiB page */
		return (void *) PT_ADDR_HUGE(tmp_ptr[pdp_idx])
			+ (virt_addr & 0x3fffffffULL);

	tmp_ptr = (void *) PT_ADDR(tmp_ptr[pdp_idx]); /* Page directory */

	/* Page directory */
	if (!(tmp_ptr[pd_idx] & 1))
		return NULL;
	else if (tmp_ptr[pd_idx] & 0x80) /* 2 MiB page */
		return (void *) PT_ADDR_HUGE(tmp_ptr[pd_idx])
			+ (virt_addr & 0x1fffffULL);

	tmp_ptr = (void *) PT_ADDR(tmp_ptr[pd_idx]);  /* Page table */

	/* Page table */
	if (!(tmp_ptr[pt_idx] & 1))
		return NULL;
	else /* 4 KiB page */
		return (void *) PT_ADDR(tmp_ptr[pt_idx]) + (virt_addr & 0xfffULL);
}
#endif

uint8_t sipi_core = 0;
uint8_t sipi_vector = 0;

#define VMEXIT_EXP_DB	0x41
#define VMEXIT_EXP_SX	0x5e
#define VMEXIT_CPUID	0x72
#define VMEXIT_VMRUN	0x80
#define VMEXIT_VMMCALL	0x81
#define VMEXIT_NPF	0x400

void
vmexit_handler(struct vmm_cpu *ctx)
{
	uint64_t rax, rcx, rdx, rbx;
	uint8_t *guest_rip;
	uint32_t val;

	switch (ctx->vmcb.exitcode) {
	case VMEXIT_CPUID:
		/* Print CPUID leaf and subleaf */
		// uart_print("CPUID RAX=%lx, RCX=%lx\n", ctx->vmcb.rax,ctx->gprs.rcx);

		rax = ctx->vmcb.rax;
		rcx = ctx->gprs.rcx;
		asm volatile (
			"movq %0, %%rax\n"
			"movq %1, %%rcx\n"
			"cpuid\n"
			"movq %%rax, %0\n"
			"movq %%rcx, %1\n"
			"movq %%rdx, %2\n"
			"movq %%rbx, %3\n"
			: "=m" (rax), "=m" (rcx), "=m" (rdx), "=m" (rbx)
			:: "rax", "rcx", "rdx", "rbx");

		switch (ctx->vmcb.rax) {
		// case 0:	/* Change the CPUID string to BootlegAMD */
		// 	ctx->vmcb.rax = rax;
		// 	ctx->gprs.rbx = 0x746f6f42;
		// 	ctx->gprs.rcx = 0x0000444d;
		// 	ctx->gprs.rdx = 0x4167656c;
		// 	break;
		case 1:	/* Hide x2APIC support from the OS */
			ctx->vmcb.rax = rax;
			ctx->gprs.rbx = rbx;
			ctx->gprs.rcx = rcx & ~(1 << 21);
			ctx->gprs.rdx = rdx;
			break;
		default:
			ctx->vmcb.rax = rax;
			ctx->gprs.rbx = rbx;
			ctx->gprs.rcx = rcx;
			ctx->gprs.rdx = rdx;
			break;
		}
		ctx->vmcb.rip += 2;
		break;
	case VMEXIT_VMMCALL: /* AP guest code fires this
				when the BSP sends a SIPI */
		uart_print("VMMCALL\n");

		/* Switch to real mode */
		ctx->vmcb.cr0 = 0;
		ctx->vmcb.cr3 = 0;
		ctx->vmcb.cr4 = 0;
		ctx->vmcb.efer = EFER_SVME;

		/* Start at the kernel's trampoline */
		ctx->vmcb.cs_selector = (uint16_t) sipi_vector << 8;
		ctx->vmcb.cs_base = sipi_vector << 12;
		ctx->vmcb.cs_limit = 0xffff;
		ctx->vmcb.cs_attrib = 0x9b;
		ctx->vmcb.rip = 0;

		ctx->vmcb.ds_selector = 0;
		ctx->vmcb.ds_base = 0;
		ctx->vmcb.ds_limit = 0xffff;
		ctx->vmcb.ds_attrib = 0x93;
		ctx->vmcb.es_selector = 0;
		ctx->vmcb.es_base = 0;
		ctx->vmcb.es_limit = 0xffff;
		ctx->vmcb.es_attrib = 0x93;
		ctx->vmcb.ss_selector = 0;
		ctx->vmcb.ss_base = 0;
		ctx->vmcb.ss_limit = 0xffff;
		ctx->vmcb.ss_attrib = 0x93;
		ctx->vmcb.fs_selector = 0;
		ctx->vmcb.fs_base = 0;
		ctx->vmcb.fs_limit = 0xffff;
		ctx->vmcb.fs_attrib = 0x93;
		ctx->vmcb.gs_selector = 0;
		ctx->vmcb.gs_base = 0;
		ctx->vmcb.gs_limit = 0xffff;
		ctx->vmcb.gs_attrib = 0x93;

		break;
	case VMEXIT_NPF:
		/* Store target LAPIC register */
		ctx->lapic_reg = ctx->vmcb.exitinfo2 - (uint64_t) lapic_addr;

		/* Switch to emulator page */
		ctx->lapic_pt[0] = (uint64_t) ctx->lapic_emu | 7;

		/* Set trap flag */
		ctx->vmcb.rflags |= (1 << 8);

		break;
	case VMEXIT_EXP_DB:
		/* Get the value the guest wrote */
		val = *(uint32_t *) (ctx->lapic_emu + ctx->lapic_reg);

		if (ctx->lapic_reg == 0x300) {
			if ((val >> 8 & 0xf) == 5) {
				uart_print("INIT apic: %p\n",
					(*(uint32_t *) 0xfee00310) >> 24);
			} else if ((val >> 8 & 0xf) == 6) {
				sipi_vector = val & 0xff;
				sipi_core = (*(uint32_t *) 0xfee00310) >> 24;
				uart_print("SIPI apic: %p, vector: %p\n",
					sipi_core, sipi_vector);
			} else {
				*(uint32_t *) (lapic_addr + ctx->lapic_reg) = val;
			}
		} else {
			*(uint32_t *) (lapic_addr + ctx->lapic_reg) = val;
		}

		/* Switch to real LAPIC */
		ctx->lapic_pt[0] = (uint64_t) lapic_addr | 5;

		/* Clear trap flag */
		ctx->vmcb.rflags &= ~(1 << 8);

		break;
	default:
		uart_print("Unknown #VMEXIT %p %p %p\n",
			ctx->vmcb.exitcode, ctx->vmcb.exitinfo1, ctx->vmcb.exitinfo2);
		for (;;)
			;
		break;
	}
}
