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
 * Global kernel lock
 * NOTE: the BSP owns the lock from the start
 */
int kernel_global_lock = 1;

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
	kernel_pml4 = alloc_pages(1, 0xffffffff);
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

static
uint64_t
gdt[] = {
	0,
	0x00209A0000000000,	/* __BOOT_CS */
	0x0000920000000000,	/* __BOOT_DS */
};

void
kernel_core_init(void)
{
	struct {
		uint16_t limit;
		uint64_t addr;
	} __attribute__((packed)) gdtr;

	gdtr.limit = sizeof(gdt);
	gdtr.addr = (uint64_t) gdt;

	asm volatile ("lgdt %0\n"
			"pushq $0x08\n"
			"pushq $reload_cs\n"
			"retfq; reload_cs:\n"
			"movl $0x10, %%eax\n"
			"movl %%eax, %%ds\n"
			"movl %%eax, %%es\n"
			"movl %%eax, %%ss\n"
			"movl %%eax, %%fs\n"
			"movl %%eax, %%gs"
			 :: "m" (gdtr) : "rax");

	gdtr.limit = 0;
	gdtr.addr = 0;

	asm volatile ("lidt %0" :: "m" (gdtr));
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

	/* Use our own GDT */
	kernel_core_init();
	uart_print("Kernel GDT and IDT loaded!\n");

	/* SMP init depends on lowmem + page table */
	acpi_smp_init((acpi_rsdp *) handover->rsdp_addr);

	/* Start Linux in the VMM */
	uart_print("Calling BSP VMM startup!\n");
	vmm_startup_bsp(vmm_setup_core(), handover);
}
