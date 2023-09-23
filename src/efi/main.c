/*
 * Entry point from UEFI
 */

#include <efi.h>
#include <include/handover.h>
#include <kernel/string.h>

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
efi_status_t (efiapi *exit_boot_services_orig)(efi_handle_t image_handle, efi_size_t map_key);

/*
 * Hook written in assembly
 */
efi_status_t efiapi exit_boot_services_hook(efi_handle_t image_handle, efi_size_t map_key);

/*
 * GUID for identifying the RSDP configuration table
 */
#define EFI_ACPI_TABLE_GUID \
  { 0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

efi_status_t efiapi efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
  efi_status_t status;
  efi_loaded_image_protocol_t *loaded_image;

  /* Make sure we are loaded under 4GiB */
  status = system_table->boot_services->handle_protocol(image_handle,
    &(efi_guid_t) EFI_LOADED_IMAGE_PROTOCOL_GUID, (void **) &loaded_image);
  if (EFI_ERROR(status))
    return status;
  if ((efi_u64_t) loaded_image->image_base
      + loaded_image->image_size > 0x10000000) {
    system_table->con_out->output_string(system_table->con_out,
      L"Must be loaded under 4GiB!\r\n");
    return EFI_UNSUPPORTED;
  }

  /* Allocate handover block */
  status = system_table->boot_services->allocate_pages(
    EFI_ALLOCATE_ANY_PAGES,
    EFI_RUNTIME_SERVICES_DATA,
    PAGE_COUNT(sizeof(struct grr_handover)),
    (efi_physical_address_t *) &handover);
  if (EFI_ERROR(status))
    return status;

  /* Allocate hypervisor memory blocks */
  handover->hmem_entries = 2;

  handover->hmem[0].addr = 0x100000;  /* 64K of low-memory */
  handover->hmem[0].size = 0x10000;
  status = system_table->boot_services->allocate_pages(
    EFI_ALLOCATE_MAX_ADDRESS,
    EFI_RUNTIME_SERVICES_DATA,
    PAGE_COUNT(handover->hmem[0].size),
    (efi_physical_address_t *) &handover->hmem[0].addr);
  if (EFI_ERROR(status))
    return status;

  handover->hmem[1].size = 0x1000000;
  status = system_table->boot_services->allocate_pages(   /* 16M of high-memory */
    EFI_ALLOCATE_ANY_PAGES,
    EFI_RUNTIME_SERVICES_DATA,
    PAGE_COUNT(handover->hmem[1].size),
    (efi_physical_address_t *) &handover->hmem[1].addr);
  if (EFI_ERROR(status))
    return status;

  /* Find APCI RSDP */
  for (size_t i = 0; i < system_table->cnt_config_entries; ++i)
    if (!memcmp(&system_table->config_entries[i].vendor_guid,
        &(efi_guid_t) EFI_ACPI_TABLE_GUID, sizeof (efi_guid_t)))
      handover->rsdp_addr = (efi_u64_t) system_table->config_entries[i].vendor_table;

  /* Hook exit_boot_services */
  exit_boot_services_orig = system_table->boot_services->exit_boot_services;
  system_table->boot_services->exit_boot_services = &exit_boot_services_hook;

  return status;
}
