/*
 * Entry point from UEFI
 */

#include <efi.h>
#include <efiutil.h>
#include <khelper.h>
#include "elf64.h"
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
 * Core kernel entry point
 */
void *kernel_init;

static
efi_status
locate_self_volume(efi_simple_file_system_protocol **self_volume)
{
	efi_status status;
	efi_loaded_image_protocol *self_loaded_image;

	status = bs->handle_protocol(
		self_image_handle,
		&(efi_guid) EFI_LOADED_IMAGE_PROTOCOL_GUID,
		(void **) &self_loaded_image);
	if (EFI_ERROR(status))
		return status;

	status = bs->handle_protocol(
		self_loaded_image->device_handle,
		&(efi_guid) EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
		(void **) self_volume);
	return status;
}

static
efi_status
read_file(efi_file_protocol *file, efi_ssize offs, efi_size size, void *buffer)
{
	efi_status status;
	efi_size outsize;

	if (offs >= 0) {
		status = file->set_position(file, offs);
		if (EFI_ERROR(status))
			return status;
	}

	outsize = size;
	status = file->read(file, &outsize, buffer);
	if (EFI_ERROR(status))
		return status;

	return status;
}

efi_status
load_core(efi_ch16 *path)
{
	efi_status status;
	efi_simple_file_system_protocol *vol;
	efi_file_protocol *root, *file;

	struct elf64_hdr ehdr;
	efi_size phdr_idx;
	struct elf64_phdr phdr;
	void *pbase;

	root = NULL;
	file = NULL;

	status = locate_self_volume(&vol);
	if (EFI_ERROR(status))
		goto done;
	status = vol->open_volume(vol, &root);
	if (EFI_ERROR(status))
		goto done;
	status = root->open(root, &file, path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status))
		goto done;

	/* Read file header */
	status = read_file(file, 0, sizeof(ehdr), &ehdr);
	if (EFI_ERROR(status))
		goto done;

	/* Prevent stack overflows */
	if (ehdr.e_phentsize > sizeof(struct elf64_phdr)) {
		status = EFI_INVALID_PARAMETER;
		goto done;
	}

	/* Read program headers */
	for (phdr_idx = 0; phdr_idx < ehdr.e_phnum; ++phdr_idx) {
		status = read_file(file,
			ehdr.e_phoff + ehdr.e_phentsize * phdr_idx,
			ehdr.e_phentsize,
			&phdr);
		if (EFI_ERROR(status))
			goto done;

		/* We only care about stuff we need to load */
		if (phdr.p_type != PT_LOAD)
			continue;

		/* Prevent heap overflows */
		if (phdr.p_filesz > phdr.p_memsz) {
			status = EFI_INVALID_PARAMETER;
			goto done;
		}

		/* Allocate space for section */
		pbase = (void *) phdr.p_paddr;
		status = bs->allocate_pages(
			allocate_address,
			efi_runtime_services_code,
			PAGE_COUNT(phdr.p_memsz),
			(efi_physical_address *) &pbase);
		if (EFI_ERROR(status))
			goto done;

		/* Zero section */
		bzero(pbase, PAGE_SIZE * PAGE_COUNT(phdr.p_memsz));

		/* Read section data */
		status = read_file(file, phdr.p_offset, phdr.p_filesz, pbase);
		if (EFI_ERROR(status))
			goto done;

		efi_print(L"Loaded paddr: %p, filesz: %p, memsz: %p\n",
			phdr.p_paddr, phdr.p_filesz, phdr.p_memsz);
	}

	/* Store entry point */
	kernel_init = (void *) ehdr.e_entry;

done:
	if (file)
		file->close(file);
	if (root)
		root->close(root);
	return status;
}

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

	status = load_core(L"grr.elf");
	if (EFI_ERROR(status))
		return status;

	/* Allocate handover block */
	status = bs->allocate_pages(
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
	status = bs->allocate_pages(
		allocate_max_address,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[0].size),
		(efi_physical_address *) &handover->hmem[0].addr);
	if (EFI_ERROR(status))
		return status;

	handover->hmem[1].size = 0x1000000;
	status = bs->allocate_pages(		/* 16M of high-memory */
		allocate_any_pages,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[1].size),
		(efi_physical_address *) &handover->hmem[1].addr);
	if (EFI_ERROR(status))
		return status;

	/* Find APCI RSDP */
	for (i = 0; i < st->cnt_config_entries; ++i)
		if (!memcmp(&st->config_entries[i].vendor_guid,
				&(efi_guid) EFI_ACPI_TABLE_GUID,
				sizeof(efi_guid)))
			handover->rsdp_addr =
				(efi_u64) st->config_entries[i].vendor_table;

	/* Hook exit_boot_services */
	exit_boot_services_orig = bs->exit_boot_services;
	bs->exit_boot_services = &exit_boot_services_hook;

	return status;
}
