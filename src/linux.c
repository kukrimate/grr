/*
 * Linux loader
 */

#include <efi.h>
#include <efiutil.h>
#include "bootparam.h"

#define PAGE_SIZE 4096
#define PAGE_COUNT(x) ((x + PAGE_SIZE - 1) / PAGE_SIZE)

static
efi_status
load_kernel(efi_ch16 *filename, void **kernel_base, efi_size *kernel_size)
{
	efi_status		status;
	efi_file_protocol	*kernel_file;
	efi_file_info		*kernel_info;

	/* Open kernel file */
	status = self_root_dir->open(
		self_root_dir,
		&kernel_file,
		filename,
		EFI_FILE_MODE_READ,
		0);
	if (EFI_ERROR(status))
		return status;

	/* Get kernel size */
	status = get_file_info(kernel_file, &kernel_info);
	if (EFI_ERROR(status))
		goto done;
	*kernel_size = kernel_info->file_size;
	free(kernel_info);

	/* Allocate memory */
	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_code,
		PAGE_COUNT(*kernel_size),
		(efi_physical_address *) kernel_base);
	if (EFI_ERROR(status))
		goto done;

	/* Load kernel */
	status = kernel_file->read(
		kernel_file,
		kernel_size,
		*kernel_base);
done:
	kernel_file->close(kernel_file);
	return status;
}

enum {
	E820_USABLE		= 1,
	E820_RESERVED		= 2,
	E820_ACPI_RECLAIM	= 3,
	E820_APCI_NVS		= 4,
	E820_UNUSABLE		= 5,
};

typedef struct boot_e820_entry e820_entry;

static
efi_status
convert_mmap(struct boot_params *boot_params, efi_size *map_key)
{
	efi_status status;

	/* UEFI memory map */
	void		*mmap;
	efi_size	mmap_size;
	efi_size	desc_size;
	efi_u32		desc_ver;
	efi_memory_descriptor *mmap_ent;

	/* E820 memory map */
	e820_entry	*e820_cur;

	status = EFI_SUCCESS;

	mmap = NULL;
	mmap_size = 0;

retry:

	status = bs->get_memory_map(
		&mmap_size,
		mmap,
		map_key,
		&desc_size,
		&desc_ver);

	if (status == EFI_BUFFER_TOO_SMALL) {
		mmap = malloc(mmap_size);
		goto retry;
	}

	if (EFI_ERROR(status)) /* Failed to get UEFI memory map */
		goto end;

	/* Allocate an E820 memory map with the same number of entries */
	e820_cur = boot_params->e820_table;

	if (mmap_size / desc_size > E820_MAX_ENTRIES_ZEROPAGE)
		abort(L"Sorry, can't boot on your crap firmware,"
		" memory map too big for Linux!\r\n", EFI_UNSUPPORTED);

	/* Convert UEFI memory map to E820 */
	for (mmap_ent = mmap; (void *) mmap_ent < mmap + mmap_size;
				mmap_ent = (void *) mmap_ent + desc_size) {
		e820_cur->addr = mmap_ent->start;
		e820_cur->size = mmap_ent->number_of_pages * PAGE_SIZE;
		switch (mmap_ent->type) {
		case efi_conventional_memory:
		case efi_loader_code:
		case efi_loader_data:
		case efi_boot_services_code:
		case efi_boot_services_data:
			e820_cur->type = E820_USABLE;
			break;
		case efi_acpi_reclaim_memory:
			e820_cur->type = E820_ACPI_RECLAIM;
			break;
		case efi_acpi_memory_nvs:
			e820_cur->type = E820_APCI_NVS;
			break;
		case efi_unusable_memory:
			e820_cur->type = E820_UNUSABLE;
			break;
		/* Default to reserved */
		default:
			e820_cur->type = E820_RESERVED;
			break;
		}
		++e820_cur;
	}

end:
	/* NOTE: the UEFI mmap cannot be freed otherwise UEFI shits itself */
	return status;
}

efi_status
boot_linux(efi_ch16 *filename, efi_ch16 *cmdline)
{

}
