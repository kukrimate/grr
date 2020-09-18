/*
 * System intialization
 */

#include <stddef.h>
#include <stdint.h>
#include <include/handover.h>
#include <kernel/acpi.h>
#include <kernel/alloc.h>
#include <kernel/kernel.h>
#include <kernel/uart.h>
#include <vmm/vmm.h>

/*
 * Paging setup code
 */

uint64_t *kernel_pml4;

static
void
pginit(void)
{
	uint64_t cur_phys, *pdp;
	size_t pml4_idx, pdp_idx;

	/* Must be <4G, it's loaded in 32-bit mode during SMP init */
	kernel_pml4 = alloc_pages(1, (void *) 0xffffffff);
	cur_phys = 0;

	for (pml4_idx = 0; pml4_idx < 256; ++pml4_idx) {
		pdp = alloc_pages(1, 0);
		for (pdp_idx = 0; pdp_idx < 512; ++pdp_idx) {
			pdp[pdp_idx] = cur_phys | 0x83;
			cur_phys += 0x40000000;
		}
		kernel_pml4[pml4_idx] = (uint64_t) pdp | 3;
	}
	asm volatile ("movq %0, %%cr3" :: "r" (kernel_pml4));
}

/*
 * Segmentation setup code
 */

uint64_t
kernel_gdt[] = {
	0,
	0x00cf9a000000ffff,	/* CS32 */
	0x00cf92000000ffff,	/* DS32 */
	0x00209A0000000000,	/* CS64 */
	0x0000920000000000,	/* DS64 */
};

uint16_t
kernel_gdt_size = sizeof(kernel_gdt);

void
load_gdt(void *base, uint16_t limit);

void
load_idt(void *base, uint16_t limit);

void
kernel_cpu_init(void)
{
	load_gdt(kernel_gdt, kernel_gdt_size);
	asm volatile ("pushq $0x18\n"
			"leaq reload_cs(%%rip), %%rax\n"
			"pushq %%rax\n"
			"retfq; reload_cs:\n"
			"movl $0x20, %%eax\n"
			"movl %%eax, %%ds\n"
			"movl %%eax, %%es\n"
			"movl %%eax, %%ss\n"
			"movl %%eax, %%fs\n"
			"movl %%eax, %%gs"
			 ::: "rax");

	load_idt(0, 0);
}


/*
 * Kernel entry point
 */
void
kernel_main(struct grr_handover *handover)
{
	/* Setup UART so we can print stuff */
	uart_setup();
	uart_print("Hypervisor kernel entry\n");

	/* Setup page allocator */
	alloc_init(handover);

	/* Use our own page table */
	pginit();
	uart_print("Kernel PML4: %p\n", kernel_pml4);

	/* Load GDT and IDT (not used for now) */
	kernel_cpu_init();
	uart_print("Kernel GDT and IDT loaded!\n");

	/* Bring up VMs on all APs */
	acpi_smp_init((acpi_rsdp *) handover->rsdp_addr);

	/* Bring up VM on the BSP */
	uart_print("Calling BSP VMM startup!\n");
	vmm_execute(vmm_setup_bsp(handover));
}
