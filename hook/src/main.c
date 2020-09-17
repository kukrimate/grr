/*
 * Entry point from UEFI
 */

#include <efi.h>
#include <efiutil.h>
#include <khelper.h>
#include "handover.h"

/*
 * Physical page size
 */
#define PAGE_SIZE 0x1000

/*
 * Number of pages needed to fit an object
 */
#define PAGE_COUNT(x) ((x + PAGE_SIZE - 1) / PAGE_SIZE)

/*
 * Handover block
 */
struct grr_handover *handover;

/*
 * Saved pointer to the firmware function
 */
efi_status
(efiapi *exit_boot_services_orig)(efi_handle image_handle, efi_size map_key);

/*
 * Hook written in assembly
 */
efi_status
efiapi
exit_boot_services_hook(efi_handle image_handle, efi_size map_key);

/*
 * GUID for identifying the RSDP configuration table
 */
#define EFI_ACPI_TABLE_GUID \
  { 0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x0, 0x80, \
  	0xc7, 0x3c, 0x88, 0x81 }

efi_status
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;
	efi_size i;

	efi_init(image_handle, system_table);

	/* Allocate handover block */
	status = system_table->boot_services->allocate_pages(
		allocate_any_pages,
		efi_runtime_services_data,
		PAGE_COUNT(sizeof(struct grr_handover)),
		(efi_physical_address *) &handover);
	if (EFI_ERROR(status))
		return status;

	/* Allocate hypervisor memory blocks */
	handover->hmem_entries = 2;

	handover->hmem[0].addr = 0x100000;	/* 64K of low-memory */
	handover->hmem[0].size = 0x10000;
	status = system_table->boot_services->allocate_pages(
		allocate_max_address,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[0].size),
		(efi_physical_address *) &handover->hmem[0].addr);
	if (EFI_ERROR(status))
		return status;

	handover->hmem[1].size = 0x1000000;
	status = system_table->boot_services->allocate_pages(		/* 16M of high-memory */
		allocate_any_pages,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[1].size),
		(efi_physical_address *) &handover->hmem[1].addr);
	if (EFI_ERROR(status))
		return status;

	/* Find APCI RSDP */
	for (i = 0; i < system_table->cnt_config_entries; ++i)
		if (!memcmp(&system_table->config_entries[i].vendor_guid,
				&(efi_guid) EFI_ACPI_TABLE_GUID,
				sizeof(efi_guid)))
			handover->rsdp_addr =
				(efi_u64) system_table->config_entries[i].vendor_table;

	/* Hook exit_boot_services */
	exit_boot_services_orig = system_table->boot_services->exit_boot_services;
	system_table->boot_services->exit_boot_services = &exit_boot_services_hook;

	return status;
}
