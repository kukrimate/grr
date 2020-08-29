/*
 * ACPI related functions
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include "acpi.h"
#include "kernel.h"
#include "uart.h"

static
void *
acpi_find_table(acpi_rsdp *rsdp, u32 signature)
{
	acpi_rsdt *rsdt;
	acpi_xsdt *xsdt;
	acpi_hdr *pointer;
	size_t i;

	if (rsdp->revision < 2) {	/* ACPI 1.0 */
		rsdt = (acpi_rsdt *) (uint64_t) rsdp->rsdt_addr;
		for (i = 0; i < (rsdt->hdr.length
				 - sizeof(acpi_hdr)) / sizeof(uint32_t); ++i) {
			pointer = (acpi_hdr *) (uint64_t) rsdt->pointers[i];
			if (pointer->signature == signature)
				return pointer;
		}
	} else {			/* ACPI 2.0+ */
		xsdt = (acpi_xsdt *) rsdp->xsdt_addr;
		for (i = 0; i < (xsdt->hdr.length
				 - sizeof(acpi_hdr)) / sizeof(uint64_t); ++i) {
			pointer = (acpi_hdr *) xsdt->pointers[i];
			if (pointer->signature == signature)
				return pointer;
		}
	}

	/* No table found */
	return NULL;
}

/* SMP init trampoline */
void smp_init16();
void smp_init16_end();

void
acpi_smp_init(acpi_rsdp *rsdp)
{
	acpi_madt	*madt;
	size_t		len;
	acpi_madt_entry	*madt_entry;
	void 		*trampoline;

	uart_print("Setting up SMP...\n");

	trampoline = kernel_lowmem_alloc(1);
	memcpy(trampoline, smp_init16, smp_init16_end - smp_init16);
	uart_print("SMP AP trampoline at: %p (Page: %d)\n",
		trampoline, (uint64_t) trampoline / 4096);

	madt = acpi_find_table(rsdp, ACPI_MADT_SIGNATURE);
	uart_print("Found MADT at: %p\n", madt);

	len = madt->hdr.length - sizeof(acpi_madt);
	madt_entry = madt->entries;
	while (len) {
		/* Look for LAPIC entries */
		if (!madt_entry->type && madt_entry->lapic.apic_id) {
			uart_print("Waking up CPU: %d, APIC: %d\n",
				madt_entry->lapic.cpu_id,
				madt_entry->lapic.apic_id);

			/* Init IPI */
			*(uint32_t *) (uint64_t) (madt->lapic_addr + 0x310) =
			(madt_entry->lapic.apic_id << 24);
			*(uint32_t *) (uint64_t) (madt->lapic_addr + 0x300) =
			0x00004500;

			/* Startup IPI */
			*(uint32_t *) (uint64_t) (madt->lapic_addr + 0x310) =
				(madt_entry->lapic.apic_id << 24);
			*(uint32_t *) (uint64_t) (madt->lapic_addr + 0x300) =
				0x00004600 | ((uint64_t) trampoline / 4096);
		}

		len -= madt_entry->length;
		madt_entry = (void *) madt_entry + madt_entry->length;
	}
}
