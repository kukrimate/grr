/*
 * GRR main file
 */

#include <efi.h>
#include <efiutil.h>

efi_status
boot_linux(efi_ch16 *filename, const char *cmdline);

void
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;

	init_util(image_handle, system_table);
	status = boot_linux(L"\\efi\\grr\\vmlinuz-4.19.0-10-amd64",
		"earlyprintk=serial,ttyS0,38400");
	abort(L"Failed to boot Linux!\r\n", status);
}
