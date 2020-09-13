/*
 * ACPI related functions
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include <kernel/acpi.h>
#include <kernel/alloc.h>
#include <kernel/kernel.h>
#include <kernel/uart.h>
#include <vmm/vmm.h>

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

/* The LAPIC address as discovered from the MADT */
void *lapic_addr;

/* SMP init trampoline */
void smp_init16();
void smp_init16_end();

/* Stack pointer for the AP */
void smp_init64_rsp();

void
acpi_smp_init(acpi_rsdp *rsdp)
{
	acpi_madt	*madt;
	size_t		len;
	acpi_madt_entry	*madt_entry;
	void 		*trampoline;

	uart_print("Setting up SMP...\n");

	trampoline = alloc_pages(1, (void *) 0x100000); /* This must be <1M */
	memcpy(trampoline, smp_init16, smp_init16_end - smp_init16);
	uart_print("SMP AP trampoline at: %p (Page: %d)\n",
		trampoline, (uint64_t) trampoline / 4096);

	madt = acpi_find_table(rsdp, ACPI_MADT_SIGNATURE);
	uart_print("Found MADT at: %p\n", madt);
	uart_print("BSP APIC ID: %d\n", acpi_get_apic_id());

	lapic_addr = (void *) (uint64_t) madt->lapic_addr;
	len = madt->hdr.length - sizeof(acpi_madt);
	madt_entry = madt->entries;
	while (len) {
		/* Look for LAPIC entries */
		if (!madt_entry->type
				&& madt_entry->lapic.apic_id
				!= acpi_get_apic_id()
				&& (madt_entry->lapic.flags & 1
					|| madt_entry->lapic.flags & 3)) {
			uart_print("Waking up AP with APIC ID: %d\n",
				madt_entry->lapic.apic_id);

			/* 4K stack for each AP */
			*(uint64_t *) (smp_init64_rsp + 2) =
				(uint64_t) alloc_pages(1, 0) + 4096;

			/* Init IPI */
			*(uint32_t *) (lapic_addr + 0x310) =
				(madt_entry->lapic.apic_id << 24);
			*(uint32_t *) (lapic_addr + 0x300) = 0x00004500;

			/* Startup IPI */
			*(uint32_t *) (lapic_addr + 0x310) =
				(madt_entry->lapic.apic_id << 24);
			*(uint32_t *) (lapic_addr + 0x300) =
				0x00004600 | ((uint64_t) trampoline / 4096);

			/* Wait for the newly started AP to finish */
			spinlock_unlock(kernel_global_lock);
			for (size_t i =0; i < 100000000; ++i)
				;
			spinlock_lock(kernel_global_lock);
		}

		len -= madt_entry->length;
		madt_entry = (void *) madt_entry + madt_entry->length;
	}

	uart_print("SMP setup done!\n");
}

int
acpi_get_apic_id(void)
{
	return  *(uint32_t *) (lapic_addr + 0x20) >> 24;
}

/*
 * C entry point for APs
 */
void
acpi_smp_ap_entry(void)
{
	kernel_core_init();
	spinlock_lock(kernel_global_lock);
	uart_print("Calling AP VMM startup!\n");
	vmm_execute(vmm_setup_ap());
}
