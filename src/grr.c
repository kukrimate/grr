/*
 * GRR main file
 */

#include <efi.h>
#include <efiutil.h>

#define KERNEL_FILENAME	L"\\efi\\grr\\vmlinuz-4.19.0-10-amd64"
#define KERNEL_CMDLINE	"earlyprintk=serial,ttyS0,38400"

efi_status
boot_linux(efi_ch16 *filename, efi_ch16 *cmdline);

void
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;

	void			*kernel_base;
	efi_size		kernel_size;

	struct boot_params	*boot_params;

	status = EFI_SUCCESS;
	init_util(image_handle, system_table);

	status =

	/* Hang so the output is visible */
	for (;;)
		;
}
