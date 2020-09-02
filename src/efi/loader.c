/*
 * Linux loader
 */

#include <efi.h>
#include <efiutil.h>
#include <khelper.h>
#include <include/bootparam.h>
#include <include/handover.h>

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

typedef struct boot_e820_entry e820_entry;

static
efi_status
convert_mmap(struct boot_params *boot_params, efi_size *map_key)
{
	efi_status status;
	efi_loaded_image_protocol *loaded_image;

	/* UEFI memory map */
	void		*mmap;
	efi_size	mmap_size;
	efi_size	desc_size;
	efi_u32		desc_ver;
	efi_memory_descriptor *mmap_ent;

	/* E820 memory map */
	efi_u32		e820_last_type;
	efi_size	e820_entries;
	e820_entry	*e820_cur;

	status = bs->handle_protocol(
		self_image_handle,
		&(efi_guid) EFI_LOADED_IMAGE_PROTOCOL_GUID,
		(void **) &loaded_image);
	if (EFI_ERROR(status))
		return status;

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
		mmap = efi_alloc(mmap_size);
		goto retry;
	}

	if (EFI_ERROR(status)) /* Failed to get UEFI memory map */
		return status;

	/* Allocate an E820 memory map with the same number of entries */
	e820_cur = boot_params->e820_table;
	e820_entries = mmap_size / desc_size;
	/* Make sure this firmware's memory map is compatible with the kernel */
	if (e820_entries > E820_MAX_ENTRIES_ZEROPAGE) {
		efi_free(mmap);
		return EFI_UNSUPPORTED;
	}
	boot_params->e820_entries = e820_entries;

	/* Convert UEFI memory map to E820 */
	e820_last_type = 0;
	for (mmap_ent = mmap; (void *) mmap_ent < mmap + mmap_size;
				mmap_ent = (void *) mmap_ent + desc_size) {
		/* Make sure the kernel won't trash us */
		if (mmap_ent->start == (efi_size) loaded_image->image_base)
			mmap_ent->type = efi_runtime_services_code;

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

		/* Merge subsequent regions with the same type */
		if (e820_cur->type == e820_last_type &&
				e820_cur[-1].addr +
				e820_cur[-1].size == e820_cur->addr) {
			e820_cur[-1].size += e820_cur->size;
			--boot_params->e820_entries;
		} else {
			e820_last_type = e820_cur->type;
			++e820_cur;
		}
	}

	/* NOTE: the UEFI mmap cannot be freed otherwise UEFI shits itself */
	return status;
}

#if 0
static
void
print_e820_mmap(efi_size entries, e820_entry *mmap)
{
	while (entries--) {
		efi_print(L"%d\t%p-%p\n", mmap->type, mmap->addr,
			mmap->addr + mmap->size - 1);
		++mmap;
	}
}
#endif

static
efi_status
setup_video(struct boot_params *boot_params)
{
	efi_status	status;
	efi_size	handle_cnt;
	efi_handle	*handles;

	efi_graphics_output_protocol *gop;
	efi_graphics_output_mode_information *mode_info;

	/* Locate all GOP hanldes */
	handles = NULL;
	status = locate_all_handles(
		&(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
		&handle_cnt,
		&handles);

	if (EFI_ERROR(status))
		goto end;

	if (!handle_cnt) {
		status = EFI_UNSUPPORTED;
		goto end;
	}

	/* We give the first framebuffer to Linux */
	status = bs->handle_protocol(
		handles[0],
		&(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
		(void **) &gop);

	if (EFI_ERROR(status))
		goto end;

	mode_info = gop->mode->info;
	switch (mode_info->pixel_format) {
	case pixel_red_green_blue_reserved_8_bit_per_color:
		boot_params->screen_info.lfb_depth = 32;
		boot_params->screen_info.red_size = 8;
		boot_params->screen_info.red_pos = 0;
		boot_params->screen_info.green_size = 8;
		boot_params->screen_info.green_pos = 8;
		boot_params->screen_info.blue_size = 8;
		boot_params->screen_info.blue_pos = 16;
		boot_params->screen_info.rsvd_size = 8;
		boot_params->screen_info.rsvd_pos = 24;
		boot_params->screen_info.lfb_linelength =
			mode_info->pixels_per_scan_line * 4;
		break;
	case pixel_blue_green_red_reserved_8_bit_per_color:
		boot_params->screen_info.lfb_depth = 32;
		boot_params->screen_info.red_size = 8;
		boot_params->screen_info.red_pos = 16;
		boot_params->screen_info.green_size = 8;
		boot_params->screen_info.green_pos = 8;
		boot_params->screen_info.blue_size = 8;
		boot_params->screen_info.blue_pos = 0;
		boot_params->screen_info.rsvd_size = 8;
		boot_params->screen_info.rsvd_pos = 24;
		boot_params->screen_info.lfb_linelength =
			mode_info->pixels_per_scan_line * 4;
		break;
	default: /* TODO: add more pixel formats */
		status = EFI_UNSUPPORTED;
		goto end;
	}

	boot_params->screen_info.orig_video_isVGA = 0x70; /* EFI framebuffer */
	boot_params->screen_info.lfb_base =
		(efi_u32) (efi_u64) gop->mode->frame_buffer_base;
	boot_params->screen_info.ext_lfb_base =
		(efi_u32) ((efi_u64) gop->mode->frame_buffer_base >> 32);
	boot_params->screen_info.lfb_size = gop->mode->frame_buffer_size;
	boot_params->screen_info.lfb_width = mode_info->horizontal_resolution;
	boot_params->screen_info.lfb_height = mode_info->vertical_resolution;
	boot_params->screen_info.pages = 1;

end:
	if (handles)
		efi_free(handles);
	return status;
}

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

#define EFI_ACPI_TABLE_GUID \
  { 0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x0, 0x80, \
  	0xc7, 0x3c, 0x88, 0x81 }

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

static
efi_status
get_file_size(efi_file_protocol *file, efi_size *file_size)
{
	efi_status	status;
	efi_file_info	*file_info;

	status = get_file_info(file, &file_info);
	if (EFI_ERROR(status))
		return status;

	*file_size = file_info->file_size;
	efi_free(file_info);
	return status;
}

void
kernel_init(struct grr_handover *handover);

efi_status
boot_linux(efi_ch16 *kernel_path, efi_ch16 *initrd_path, char *cmdline)
{
	efi_status		status;

	efi_size		cmdline_size;
	struct boot_params	*boot_params;

	efi_simple_file_system_protocol	*volume;
	efi_file_protocol		*root_dir;

	efi_file_protocol	*kernel_file;
	void			*kernel_base;

	efi_file_protocol	*initrd_file;
	efi_size		initrd_size;
	void			*initrd_base;

	struct grr_handover	*handover;

	efi_size		map_key;

	/* Allocate boot params + cmdline buffer */
	cmdline_size = strlen(cmdline) + 1;
	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_data,
		PAGE_COUNT(sizeof(struct boot_params) + cmdline_size),
		(efi_physical_address *) &boot_params);
	if (EFI_ERROR(status))
		return status;
	efi_print(L"Boot params at %p\n", boot_params);
	/* Zero boot params */
	memset(boot_params, 0, sizeof(struct boot_params));
	/* Copy cmdline */
	memcpy(&boot_params[1], cmdline, cmdline_size);

	/* Open boot volume */
	status = locate_self_volume(&volume);
	if (EFI_ERROR(status))
		goto err_free_boot_params;
	status = volume->open_volume(volume, &root_dir);
	if (EFI_ERROR(status))
		goto err_free_boot_params;

	/* Open kernel */
	status = root_dir->open(
		root_dir,
		&kernel_file,
		kernel_path,
		EFI_FILE_MODE_READ,
		0);
	if (EFI_ERROR(status))
		goto err_close_rootdir;

	/* Read setup header */
	status = read_file(kernel_file,
		0x1f1,
		sizeof(struct setup_header),
		&boot_params->hdr);
	if (EFI_ERROR(status))
		goto err_close_kernel;

	/* Enforce all assumptions made about the kernel image */
	if (boot_params->hdr.boot_flag != 0xaa55 ||
			boot_params->hdr.header != 0x53726448 ||
			boot_params->hdr.version < 0x02c ||
			!boot_params->hdr.relocatable_kernel ||
			!(boot_params->hdr.xloadflags & XLF_KERNEL_64) ||
			!(boot_params->hdr.xloadflags
				& XLF_CAN_BE_LOADED_ABOVE_4G)) {
		status = EFI_INVALID_PARAMETER;
		goto err_close_kernel;
	}

	/* Allocate buffer for the kernel image */
	efi_print(L"Kernel alingment: %p\n", boot_params->hdr.kernel_alignment);
	status = alloc_aligned(
		boot_params->hdr.kernel_alignment,
		boot_params->hdr.init_size,
		&kernel_base);
	if (EFI_ERROR(status))
		goto err_close_kernel;
	efi_print(L"Kernel will be loaded at: %p\n", kernel_base);

	/* Load kernel */
	status = read_file(kernel_file,
		(boot_params->hdr.setup_sects + 1) * 512,
		boot_params->hdr.init_size,
		kernel_base);
	if (EFI_ERROR(status))
		goto err_free_kernel;

	/*
	 * Load initrd
	 */
	status = root_dir->open(
		root_dir,
		&initrd_file,
		initrd_path,
		EFI_FILE_MODE_READ,
		0);
	if (EFI_ERROR(status))
		goto err_free_kernel;

	status = get_file_size(initrd_file, &initrd_size);
	if (EFI_ERROR(status))
		goto err_close_initrd;

	status = bs->allocate_pages(
		allocate_any_pages,
		efi_loader_data,
		PAGE_COUNT(initrd_size),
		(efi_physical_address *) &initrd_base);
	if (EFI_ERROR(status))
		goto err_close_initrd;

	status = read_file(initrd_file, -1, initrd_size, initrd_base);
	if (EFI_ERROR(status))
		goto err_free_initrd;

	/*
	 * Allocate handover block
	 */
	status = bs->allocate_pages(
		allocate_any_pages,
		efi_runtime_services_data,
		PAGE_COUNT(sizeof(struct grr_handover)),
		(efi_physical_address *) &handover);
	if (EFI_ERROR(status))
		goto err_free_initrd;

	/*
	 * Allocate hypervisor memory blocks
	 */
	handover->hmem_entries = 2;

	handover->hmem[0].addr = 0x100000;	/* 64K of low-memory */
	handover->hmem[0].size = 0x10000;
	status = bs->allocate_pages(
		allocate_max_address,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[0].size),
		(efi_physical_address *) &handover->hmem[0].addr);
	if (EFI_ERROR(status))
		goto err_free_initrd;

	handover->hmem[1].size = 0x1000000;
	status = bs->allocate_pages(		/* 16M of high-memory */
		allocate_any_pages,
		efi_runtime_services_data,
		PAGE_COUNT(handover->hmem[1].size),
		(efi_physical_address *) &handover->hmem[1].addr);
	if (EFI_ERROR(status))
		goto err_free_initrd;

	/* Now we can close all file handles */
	initrd_file->close(initrd_file);
	kernel_file->close(kernel_file);
	root_dir->close(root_dir);

	/* Fake GRUB as the bootloader */
	boot_params->hdr.type_of_loader = 0xff;
	/* Make sure the kernel is *not* quiet */
	boot_params->hdr.loadflags &= ~(1 << 5);
	/* The command line is in the same buffer right after the bootparams */
	boot_params->hdr.cmd_line_ptr = (efi_u64) &boot_params[1];
	boot_params->ext_cmd_line_ptr = (efi_u64) &boot_params[1] >> 32;
	/* Set initrd address and size */
	boot_params->hdr.ramdisk_image = (efi_u64) initrd_base;
	boot_params->ext_ramdisk_image = (efi_u64) initrd_base >> 32;
	boot_params->hdr.ramdisk_size = (efi_u64) initrd_size;
	boot_params->ext_ramdisk_size = (efi_u64) initrd_size >> 32;

	/* Find ACPI tables */
	for (size_t i = 0; i < st->cnt_config_entries; ++i)
		if (!memcmp(&st->config_entries[i].vendor_guid,
				&(efi_guid) EFI_ACPI_TABLE_GUID,
				sizeof(efi_guid)))
			boot_params->acpi_rsdp_addr =
				(efi_size) st->config_entries[i].vendor_table;

	/* Find the framebuffer */
	status = setup_video(boot_params);
	if (EFI_ERROR(status))
		efi_print(L"WARN: graphics setup failed!\n");

	/* Convert the UEFI memory map to E820 for the kernel */
	status = convert_mmap(boot_params, &map_key);
	if (EFI_ERROR(status))
		return status;
	/* Get rid of boot services */
	status = bs->exit_boot_services(self_image_handle, map_key);
	if (EFI_ERROR(status))
		return status;

	/* Hand over to the hypervisor */
	handover->rsdp_addr = boot_params->acpi_rsdp_addr;
	handover->linux_entry = (efi_u64) kernel_base + 0x200;
	handover->boot_params = (efi_u64) boot_params;
	kernel_init(handover);

err_free_initrd:
	bs->free_pages((efi_physical_address) initrd_base,
		PAGE_COUNT(initrd_size));
err_close_initrd:
	initrd_file->close(initrd_file);
err_free_kernel:
	bs->free_pages((efi_physical_address) kernel_base,
		PAGE_COUNT(boot_params->hdr.init_size));
err_close_kernel:
	kernel_file->close(kernel_file);
err_close_rootdir:
	root_dir->close(root_dir);
err_free_boot_params:
	bs->free_pages(
		(efi_physical_address) boot_params,
		PAGE_COUNT(sizeof(struct boot_params) + cmdline_size));
	return status;
}
