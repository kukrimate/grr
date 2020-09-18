/*
 * GRR core linker script
 */

ENTRY(kernel_init)

PHDRS
{
	rx PT_LOAD FILEHDR PHDRS ;
	rw PT_LOAD ;
}

SECTIONS
{
	. = 4M + SIZEOF_HEADERS;

	.text : ALIGN(4K)
	{
		*(.text)
	} : rx

	.rodata : ALIGN(4K)
	{
		*(.rodata)
	} : rx

	.data : ALIGN(4K)
	{
		*(.data)
	} : rw

	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	} : rw
}
