#ifndef HANDOVER_H
#define HANDOVER_H

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/*
 * EFI to HV handover block
 */
struct grr_handover {
	/*
	 * Memory the hypervisor can use
	 */
	u64 hmem_entries;
	struct {
		u64 addr;
		u64 size;
	} hmem[32];

	/*
	 * ACPI RSDP address
	 */
	u64 rsdp_addr;

	/*
	 * Linux entry point and boot parameters
	 */
	u64 linux_entry;
	u64 boot_params;
};

#endif
