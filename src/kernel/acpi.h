#ifndef MADT_H
#define MADT_H

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
	/* ACPI 1.0 */
	u8	signature[8];
	u8	checksum;
	u8	oem_id[6];
	u8	revision;
	u32	rsdt_addr;
	/* ACPI 2.0 */
	u32	length;
	u64	xsdt_addr;
	u8	extended_checksum;
	u8	reserved[3];
} __attribute__((packed)) acpi_rsdp;

typedef struct {
	u32	signature;
	u32	length;
	u8	revision;
	u8	checksum;
	u8	oem_id[6];
	u8	oem_table_id[8];
	u32	oem_revision;
	u32	creator_id;
	u32	creator_revision;
} __attribute__((packed)) acpi_hdr;

typedef struct {
	acpi_hdr	hdr;
	u32		pointers[0];
} __attribute__((packed)) acpi_rsdt;

typedef struct {
	acpi_hdr	hdr;
	u64		pointers[0];
} __attribute__((packed)) acpi_xsdt;

#define ACPI_MADT_SIGNATURE 0x43495041

typedef struct {
	u8	cpu_id;
	u8	apic_id;
	u32	flags;
} __attribute__((packed)) acpi_madt_lapic;

typedef struct {
	u8	type;
	u8	length;
	union {
		acpi_madt_lapic	lapic;
	};
} acpi_madt_entry;

typedef struct {
	acpi_hdr	hdr;
	u32		lapic_addr;
	u32		flags;
	acpi_madt_entry entries[0];
} __attribute__((packed)) acpi_madt;

/*
 * Local APIC address
 */
extern uint64_t lapic_addr;

void
acpi_smp_init(acpi_rsdp *rsdp);

int
acpi_get_apic_id(void);

#endif
