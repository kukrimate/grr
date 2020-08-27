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
alloc_aligned(efi_size alingment, efi_size bytes, void **buffer)
{
	efi_status status;
	efi_size off;

	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_code,
		PAGE_COUNT(alingment + bytes),
		(efi_physical_address *) buffer);

	if (EFI_ERROR(status))
		return status;

	*buffer += alingment - (efi_size) *buffer % alingment;
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
	efi_size	e820_entries;
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
		return status;

	/* Allocate an E820 memory map with the same number of entries */
	e820_cur = boot_params->e820_table;
	e820_entries = mmap_size / desc_size;

	/* Make sure this firmware's memory map is compatible with the kernel */
	if (e820_entries > E820_MAX_ENTRIES_ZEROPAGE) {
		free(mmap);
		return EFI_UNSUPPORTED;
	}

	boot_params->e820_entries = e820_entries;

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

	/* NOTE: the UEFI mmap cannot be freed otherwise UEFI shits itself */
	return status;
}

static
efi_u64
gdt[] = {
	0,
	0,
	0x00209A0000000000,	/* __BOOT_CS */
	0x0000920000000000,	/* __BOOT_DS */
};

static
void
load_gdt(void)
{
	struct {
		efi_u16 limit;
		efi_u64 addr;
	} __attribute__((packed)) gdtr;

	gdtr.limit = sizeof(gdt);
	gdtr.addr = (efi_u64) gdt;

	asm volatile ("lgdt %0\n"
			"pushq $0x10\n"
			"pushq $reload_cs\n"
			"retfq; reload_cs:\n"
			"movl $0x18, %%eax\n"
			"movl %%eax, %%ds\n"
			"movl %%eax, %%es\n"
			"movl %%eax, %%ss\n"
			"movl %%eax, %%fs\n"
			"movl %%eax, %%gs"
			 :: "m" (gdtr) : "rax");
}

efi_status
boot_linux(efi_ch16 *filename, char *cmdline)
{
	efi_status		status;
	efi_size		len;
	efi_file_protocol	*kernel_file;
	struct boot_params	*boot_params;
	efi_size		cmdline_size;
	void			*cmdline_base;
	void			*kernel_base;
	efi_size		map_key;

	/* Open kernel */
	status = self_root_dir->open(
		self_root_dir,
		&kernel_file,
		filename,
		EFI_FILE_MODE_READ,
		0);

	if (EFI_ERROR(status))
		goto err;

	/* Allocate boot params */
	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_code,
		PAGE_COUNT(sizeof(struct boot_params)),
		(efi_physical_address *) &boot_params);

	if (EFI_ERROR(status))
		goto err_close;

	print(L"Boot params at %p\n", boot_params);

	/* Zero boot params */
	bzero(boot_params, sizeof(struct boot_params));

	/* Read setup header */
	status = kernel_file->set_position(kernel_file, 0x1f1);

	if (EFI_ERROR(status))
		goto err_free_params;

	len = sizeof(struct setup_header);
	status = kernel_file->read(kernel_file, &len, &boot_params->hdr);

	if (EFI_ERROR(status))
		goto err_free_params;

	if (len != sizeof(struct setup_header))
		goto err_badkernel;

	/* Enforce all assumptions made about the kernel image */
	if (boot_params->hdr.boot_flag != 0xaa55 ||
			boot_params->hdr.header != 0x53726448 ||
			boot_params->hdr.version < 0x02c ||
			!boot_params->hdr.relocatable_kernel ||
			!(boot_params->hdr.xloadflags & XLF_KERNEL_64) ||
			!(boot_params->hdr.xloadflags
				& XLF_CAN_BE_LOADED_ABOVE_4G))
		goto err_badkernel;

	/* Allocate buffer for the kernel image */
	print(L"Kernel alingment: %p\n", boot_params->hdr.kernel_alignment);

	status = alloc_aligned(
		boot_params->hdr.kernel_alignment,
		boot_params->hdr.init_size,
		&kernel_base);

	if (EFI_ERROR(status))
		goto err_free_params;

	print(L"Kernel will be loaded at: %p\n", kernel_base);

	/* Load kernel */
	status = kernel_file->set_position(
		kernel_file,
		(boot_params->hdr.setup_sects + 1) * 512);

	if (EFI_ERROR(status))
		goto err_free_kernel;

	len = boot_params->hdr.init_size;
	status = kernel_file->read(kernel_file, &len, kernel_base);

	if (EFI_ERROR(status))
		goto err_free_kernel;

	/* Kernel command line */
	cmdline_size = ascii_strlen(cmdline) + 1;
	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_code,
		PAGE_COUNT(cmdline_size),
		(efi_physical_address *) &cmdline_base);

	if (EFI_ERROR(status))
		goto err_free_kernel;

	memcpy(cmdline_base, cmdline, cmdline_size);

	/* Fill boot parameters */
	boot_params->hdr.type_of_loader = 0xff;
	boot_params->hdr.loadflags &= ~(1 << 5); /* Make sure the kernel is *not* quiet */
	boot_params->hdr.cmdline_size = cmdline_size;
	boot_params->hdr.cmd_line_ptr = (efi_u32) (efi_u64) cmdline_base;
	boot_params->ext_cmd_line_ptr = (efi_u32) ((efi_u64) cmdline_base >> 32);

	status = convert_mmap(boot_params, &map_key);

	if (EFI_ERROR(status))
		goto err_free_cmdline;

	/* Jump to kernel */
	status = bs->exit_boot_services(self_image_handle, map_key);

	if (EFI_ERROR(status))
		goto err_free_cmdline;

	load_gdt();

	asm volatile (
		"cli\n"
		"movq %0, %%rax\n"
		"movq %1, %%rsi\n"
		"jmp *%%rax" ::
		"g" (kernel_base + 0x200),
		"g" (boot_params)
		: "rax", "rsi");

	for (;;)
		;

err_badkernel:
	status = EFI_INVALID_PARAMETER;
err_free_cmdline:
	bs->free_pages((efi_physical_address) cmdline_base,
		PAGE_COUNT(cmdline_size));
err_free_kernel:
	bs->free_pages((efi_physical_address) kernel_base,
		PAGE_COUNT(boot_params->hdr.init_size));
err_free_params:
	bs->free_pages((efi_physical_address) boot_params,
		PAGE_COUNT(sizeof(*boot_params)));
err_close:
	kernel_file->close(kernel_file);
err:
	return status;
}
