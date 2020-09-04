/*
 * Entry point from UEFI
 */

#include <efi.h>
#include <efiutil.h>

efi_status
boot_linux(
	efi_ch16 *kernel_path,
	efi_ch16 *initrd_path,
	const char *cmdline);

#define KERNEL	L"\\efi\\grr\\vmlinuz-4.19.0-10-amd64"
#define INITRD	L"\\efi\\grr\\initrd.img-4.19.0-10-amd64"
// #define CMDLINE	"root=UUID=b2e1c499-2f97-4f0b-a3a6-d356dab64705 ro"	/* KVM */
// #define CMDLINE	"root=UUID=213c36a7-f326-42bd-b19c-5e71027a8c7d ro"	/* VMWARE */
#define CMDLINE "root=UUID=902a5e98-c321-4a59-bccb-5ba60067e007	ro"	/* Desktop */

void
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;

	efi_init(image_handle, system_table);

	status = boot_linux(KERNEL, INITRD, CMDLINE);
	efi_abort(L"Failed to boot Linux!\r\n", status);
}
