/*
 * System intialization
 */

#include <stddef.h>
#include <stdint.h>
#include "acpi.h"
#include "uart.h"
#include "vmm.h"
#include "../bootparam.h"

/*
 * Low memory (<1MiB) allocator
 */

static
void *lowmem_base = NULL;

static
size_t lowmem_size = 0;

static
void *lowmem_ptr = NULL;

static
void
lowmem_init(struct boot_params *boot_params)
{
	size_t i;

	for (i = 0; i < boot_params->e820_entries; ++i)
		if (!lowmem_base && boot_params->e820_table[i].type == E820_USABLE
				&& boot_params->e820_table[i].addr < 0x100000) {
			boot_params->e820_table[i].size /= 2;
			lowmem_base = (void *) boot_params->e820_table[i].addr +
				boot_params->e820_table[i].size;
			lowmem_size = boot_params->e820_table[i].size;
		}

	uart_print("Hypervisor lowmem: %p-%p\n",
		lowmem_base, lowmem_base + lowmem_size - 1);
}

void *
kernel_lowmem_alloc(size_t pages)
{
	void *buffer;

	buffer = NULL;
	if (!lowmem_ptr)
		lowmem_ptr = lowmem_base;
	if (lowmem_ptr + pages * 4096 <= lowmem_base + lowmem_size) {
		buffer = lowmem_ptr;
		lowmem_ptr += pages * 4096;
	}
	return buffer;
}

/*
 * Paging setup code
 */

uint64_t *kernel_pml4;

static
void
pginit(void)
{
	uint64_t *pdp, cur_phys;
	size_t i;

	kernel_pml4 = kernel_lowmem_alloc(1);
	pdp = kernel_lowmem_alloc(1);
	cur_phys = 0;

	for (i = 0; i < 512; ++i) {
		pdp[i] = cur_phys | 0x83;
		cur_phys += 0x40000000;
	}
	kernel_pml4[0] = (uint64_t) pdp | 3;
	asm volatile ("movq %0, %%cr3" :: "r" (kernel_pml4));
}

/*
 * Locking code
 */

// static
int bkl = 0;

void
kernel_bkl_acquire(void)
{
	asm volatile ("1: lock btsl $0, %0; jc 1b" : "=m" (bkl));
}

void
kernel_bkl_release(void)
{
	asm volatile ("lock btrl $0, %0" : "=m" (bkl));
}

void
kernel_main(void *kernel_entry, struct boot_params *boot_params)
{
	/* Setup UART so we can print stuff */
	uart_setup();
	uart_print("Hypervisor kernel entry\n");

	/* We need memory below 1 MiB for SMP startup */
	lowmem_init(boot_params);

	/* Use our own page table */
	pginit();
	uart_print("Kernel PML4: %p\n", kernel_pml4);

	/* SMP init depends on lowmem + page table */
	kernel_bkl_acquire();
	acpi_smp_init((acpi_rsdp *) boot_params->acpi_rsdp_addr);

	/* Start the kernel in the VMM */
	// vmm_startup(kernel_entry, boot_params);
	for (;;)
		;
}
