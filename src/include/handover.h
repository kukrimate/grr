#ifndef HANDOVER_H
#define HANDOVER_H

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/*
 * Memory map entry
 */
struct hmem_entry {
	u64 addr;
	u64 size;
};

/*
 * EFI to HV handover block
 */
struct grr_handover {
	/*
	 * Memory for the hypervisor to use
	 */
	u64 hmem_entries;
	struct hmem_entry hmem[32];

	/*
	 * ACPI RSDP address
	 */
	u64 rsdp_addr;

	/*
	 * Initial guest state
	 * NOTE: the offsets below are hardcoded in hook.S
	 */
	u64 rip;	/* Offset: 0x210 */
	u64 rax;	/* Offset: 0x218 */
	u64 rcx;	/* Offset: 0x220 */
	u64 rdx;	/* Offset: 0x228 */
	u64 rbx;	/* Offset: 0x230 */
	u64 rsp;	/* Offset: 0x238 */
	u64 rbp;	/* Offset: 0x240 */
	u64 rsi;	/* Offset: 0x248 */
	u64 rdi;	/* Offset: 0x250 */
	u64  r8;	/* Offset: 0x258 */
	u64  r9;	/* Offset: 0x260 */
	u64 r10;	/* Offset: 0x268 */
	u64 r11;	/* Offset: 0x270 */
	u64 r12;	/* Offset: 0x278 */
	u64 r13;	/* Offset: 0x280 */
	u64 r14;	/* Offset: 0x288 */
	u64 r15;	/* Offset: 0x290 */
} __attribute__((packed));

#endif
