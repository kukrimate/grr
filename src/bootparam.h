/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef BOOTPARAM_H
#define BOOTPARAM_H

typedef efi_u8 __u8;
typedef efi_u16 __u16;
typedef efi_u32 __u32;
typedef efi_u64 __u64;

/* setup_data/setup_indirect types */
#define SETUP_NONE			0
#define SETUP_E820_EXT			1
#define SETUP_DTB			2
#define SETUP_PCI			3
#define SETUP_EFI			4
#define SETUP_APPLE_PROPERTIES		5
#define SETUP_JAILHOUSE			6

#define SETUP_INDIRECT			(1<<31)

/* SETUP_INDIRECT | max(SETUP_*) */
#define SETUP_TYPE_MAX			(SETUP_INDIRECT | SETUP_JAILHOUSE)

/* ram_size flags */
#define RAMDISK_IMAGE_START_MASK	0x07FF
#define RAMDISK_PROMPT_FLAG		0x8000
#define RAMDISK_LOAD_FLAG		0x4000

/* loadflags */
#define LOADED_HIGH	(1<<0)
#define KASLR_FLAG	(1<<1)
#define QUIET_FLAG	(1<<5)
#define KEEP_SEGMENTS	(1<<6)
#define CAN_USE_HEAP	(1<<7)

/* xloadflags */
#define XLF_KERNEL_64			(1<<0)
#define XLF_CAN_BE_LOADED_ABOVE_4G	(1<<1)
#define XLF_EFI_HANDOVER_32		(1<<2)
#define XLF_EFI_HANDOVER_64		(1<<3)
#define XLF_EFI_KEXEC			(1<<4)
#define XLF_5LEVEL			(1<<5)
#define XLF_5LEVEL_ENABLED		(1<<6)

/* extensible setup data list node */
struct setup_data {
	__u64 next;
	__u32 type;
	__u32 len;
	__u8 data[0];
};

/* extensible setup indirect data node */
struct setup_indirect {
	__u32 type;
	__u32 reserved;  /* Reserved, must be set to zero. */
	__u64 len;
	__u64 addr;
};

struct setup_header {
	__u8	setup_sects;
	__u16	root_flags;
	__u32	syssize;
	__u16	ram_size;
	__u16	vid_mode;
	__u16	root_dev;
	__u16	boot_flag;
	__u16	jump;
	__u32	header;
	__u16	version;
	__u32	realmode_swtch;
	__u16	start_sys_seg;
	__u16	kernel_version;
	__u8	type_of_loader;
	__u8	loadflags;
	__u16	setup_move_size;
	__u32	code32_start;
	__u32	ramdisk_image;
	__u32	ramdisk_size;
	__u32	bootsect_kludge;
	__u16	heap_end_ptr;
	__u8	ext_loader_ver;
	__u8	ext_loader_type;
	__u32	cmd_line_ptr;
	__u32	initrd_addr_max;
	__u32	kernel_alignment;
	__u8	relocatable_kernel;
	__u8	min_alignment;
	__u16	xloadflags;
	__u32	cmdline_size;
	__u32	hardware_subarch;
	__u64	hardware_subarch_data;
	__u32	payload_offset;
	__u32	payload_length;
	__u64	setup_data;
	__u64	pref_address;
	__u32	init_size;
	__u32	handover_offset;
	__u32	kernel_info_offset;
} __attribute__((packed));

struct sys_desc_table {
	__u16 length;
	__u8  table[14];
};

/* Gleaned from OFW's set-parameters in cpu/x86/pc/linux.fth */
struct olpc_ofw_header {
	__u32 ofw_magic;	/* OFW signature */
	__u32 ofw_version;
	__u32 cif_handler;	/* callback into OFW */
	__u32 irq_desc_table;
} __attribute__((packed));

struct efi_info {
	__u32 efi_loader_signature;
	__u32 efi_systab;
	__u32 efi_memdesc_size;
	__u32 efi_memdesc_version;
	__u32 efi_memmap;
	__u32 efi_memmap_size;
	__u32 efi_systab_hi;
	__u32 efi_memmap_hi;
};

/*
 * This is the maximum number of entries in struct boot_params::e820_table
 * (the zeropage), which is part of the x86 boot protocol ABI:
 */
#define E820_MAX_ENTRIES_ZEROPAGE 128

/*
 * The E820 memory region entry of the boot protocol ABI:
 */
struct boot_e820_entry {
	__u64 addr;
	__u64 size;
	__u32 type;
} __attribute__((packed));

/*
 * Smallest compatible version of jailhouse_setup_data required by this kernel.
 */
#define JAILHOUSE_SETUP_REQUIRED_VERSION	1

/*
 * The boot loader is passing platform information via this Jailhouse-specific
 * setup data structure.
 */
struct jailhouse_setup_data {
	struct {
		__u16	version;
		__u16	compatible_version;
	} __attribute__((packed)) hdr;
	struct {
		__u16	pm_timer_address;
		__u16	num_cpus;
		__u64	pci_mmconfig_base;
		__u32	tsc_khz;
		__u32	apic_khz;
		__u8	standard_ioapic;
		__u8	cpu_ids[255];
	} __attribute__((packed)) v1;
	struct {
		__u32	flags;
	} __attribute__((packed)) v2;
} __attribute__((packed));

/* Copy pasted from other headers to make this build */
struct apm_bios_info {
	__u16	version;
	__u16	cseg;
	__u32	offset;
	__u16	cseg_16;
	__u16	dseg;
	__u16	flags;
	__u16	cseg_len;
	__u16	cseg_16_len;
	__u16	dseg_len;
};

struct screen_info {
	__u8  orig_x;		/* 0x00 */
	__u8  orig_y;		/* 0x01 */
	__u16 ext_mem_k;	/* 0x02 */
	__u16 orig_video_page;	/* 0x04 */
	__u8  orig_video_mode;	/* 0x06 */
	__u8  orig_video_cols;	/* 0x07 */
	__u8  flags;		/* 0x08 */
	__u8  unused2;		/* 0x09 */
	__u16 orig_video_ega_bx;/* 0x0a */
	__u16 unused3;		/* 0x0c */
	__u8  orig_video_lines;	/* 0x0e */
	__u8  orig_video_isVGA;	/* 0x0f */
	__u16 orig_video_points;/* 0x10 */

	/* VESA graphic mode -- linear frame buffer */
	__u16 lfb_width;	/* 0x12 */
	__u16 lfb_height;	/* 0x14 */
	__u16 lfb_depth;	/* 0x16 */
	__u32 lfb_base;		/* 0x18 */
	__u32 lfb_size;		/* 0x1c */
	__u16 cl_magic, cl_offset; /* 0x20 */
	__u16 lfb_linelength;	/* 0x24 */
	__u8  red_size;		/* 0x26 */
	__u8  red_pos;		/* 0x27 */
	__u8  green_size;	/* 0x28 */
	__u8  green_pos;	/* 0x29 */
	__u8  blue_size;	/* 0x2a */
	__u8  blue_pos;		/* 0x2b */
	__u8  rsvd_size;	/* 0x2c */
	__u8  rsvd_pos;		/* 0x2d */
	__u16 vesapm_seg;	/* 0x2e */
	__u16 vesapm_off;	/* 0x30 */
	__u16 pages;		/* 0x32 */
	__u16 vesa_attributes;	/* 0x34 */
	__u32 capabilities;     /* 0x36 */
	__u32 ext_lfb_base;	/* 0x3a */
	__u8  _reserved[2];	/* 0x3e */
} __attribute__((packed));

struct ist_info {
	__u32 signature;
	__u32 command;
	__u32 event;
	__u32 perf_level;
};

struct edid_info {
	unsigned char dummy[128];
};


#define EDD_MBR_SIG_MAX 16        /* max number of signatures to store */
#define EDDMAXNR 6		/* number of edd_info structs starting at EDDBUF  */

struct edd_device_params {
	__u16 length;
	__u16 info_flags;
	__u32 num_default_cylinders;
	__u32 num_default_heads;
	__u32 sectors_per_track;
	__u64 number_of_sectors;
	__u16 bytes_per_sector;
	__u32 dpte_ptr;		/* 0xFFFFFFFF for our purposes */
	__u16 key;		/* = 0xBEDD */
	__u8 device_path_info_length;	/* = 44 */
	__u8 reserved2;
	__u16 reserved3;
	__u8 host_bus_type[4];
	__u8 interface_type[8];
	union {
		struct {
			__u16 base_address;
			__u16 reserved1;
			__u32 reserved2;
		} __attribute__ ((packed)) isa;
		struct {
			__u8 bus;
			__u8 slot;
			__u8 function;
			__u8 channel;
			__u32 reserved;
		} __attribute__ ((packed)) pci;
		/* pcix is same as pci */
		struct {
			__u64 reserved;
		} __attribute__ ((packed)) ibnd;
		struct {
			__u64 reserved;
		} __attribute__ ((packed)) xprs;
		struct {
			__u64 reserved;
		} __attribute__ ((packed)) htpt;
		struct {
			__u64 reserved;
		} __attribute__ ((packed)) unknown;
	} interface_path;
	union {
		struct {
			__u8 device;
			__u8 reserved1;
			__u16 reserved2;
			__u32 reserved3;
			__u64 reserved4;
		} __attribute__ ((packed)) ata;
		struct {
			__u8 device;
			__u8 lun;
			__u8 reserved1;
			__u8 reserved2;
			__u32 reserved3;
			__u64 reserved4;
		} __attribute__ ((packed)) atapi;
		struct {
			__u16 id;
			__u64 lun;
			__u16 reserved1;
			__u32 reserved2;
		} __attribute__ ((packed)) scsi;
		struct {
			__u64 serial_number;
			__u64 reserved;
		} __attribute__ ((packed)) usb;
		struct {
			__u64 eui;
			__u64 reserved;
		} __attribute__ ((packed)) i1394;
		struct {
			__u64 wwid;
			__u64 lun;
		} __attribute__ ((packed)) fibre;
		struct {
			__u64 identity_tag;
			__u64 reserved;
		} __attribute__ ((packed)) i2o;
		struct {
			__u32 array_number;
			__u32 reserved1;
			__u64 reserved2;
		} __attribute__ ((packed)) raid;
		struct {
			__u8 device;
			__u8 reserved1;
			__u16 reserved2;
			__u32 reserved3;
			__u64 reserved4;
		} __attribute__ ((packed)) sata;
		struct {
			__u64 reserved1;
			__u64 reserved2;
		} __attribute__ ((packed)) unknown;
	} device_path;
	__u8 reserved4;
	__u8 checksum;
} __attribute__ ((packed));

struct edd_info {
	__u8 device;
	__u8 version;
	__u16 interface_support;
	__u16 legacy_max_cylinder;
	__u8 legacy_max_head;
	__u8 legacy_sectors_per_track;
	struct edd_device_params params;
} __attribute__ ((packed));

/* The so-called "zeropage" */
struct boot_params {
	struct screen_info screen_info;			/* 0x000 */
	struct apm_bios_info apm_bios_info;		/* 0x040 */
	__u8  _pad2[4];					/* 0x054 */
	__u64  tboot_addr;				/* 0x058 */
	struct ist_info ist_info;			/* 0x060 */
	__u64 acpi_rsdp_addr;				/* 0x070 */
	__u8  _pad3[8];					/* 0x078 */
	__u8  hd0_info[16];	/* obsolete! */		/* 0x080 */
	__u8  hd1_info[16];	/* obsolete! */		/* 0x090 */
	struct sys_desc_table sys_desc_table; /* obsolete! */	/* 0x0a0 */
	struct olpc_ofw_header olpc_ofw_header;		/* 0x0b0 */
	__u32 ext_ramdisk_image;			/* 0x0c0 */
	__u32 ext_ramdisk_size;				/* 0x0c4 */
	__u32 ext_cmd_line_ptr;				/* 0x0c8 */
	__u8  _pad4[116];				/* 0x0cc */
	struct edid_info edid_info;			/* 0x140 */
	struct efi_info efi_info;			/* 0x1c0 */
	__u32 alt_mem_k;				/* 0x1e0 */
	__u32 scratch;		/* Scratch field! */	/* 0x1e4 */
	__u8  e820_entries;				/* 0x1e8 */
	__u8  eddbuf_entries;				/* 0x1e9 */
	__u8  edd_mbr_sig_buf_entries;			/* 0x1ea */
	__u8  kbd_status;				/* 0x1eb */
	__u8  secure_boot;				/* 0x1ec */
	__u8  _pad5[2];					/* 0x1ed */
	/*
	 * The sentinel is set to a nonzero value (0xff) in header.S.
	 *
	 * A bootloader is supposed to only take setup_header and put
	 * it into a clean boot_params buffer. If it turns out that
	 * it is clumsy or too generous with the buffer, it most
	 * probably will pick up the sentinel variable too. The fact
	 * that this variable then is still 0xff will let kernel
	 * know that some variables in boot_params are invalid and
	 * kernel should zero out certain portions of boot_params.
	 */
	__u8  sentinel;					/* 0x1ef */
	__u8  _pad6[1];					/* 0x1f0 */
	struct setup_header hdr;    /* setup header */	/* 0x1f1 */
	__u8  _pad7[0x290-0x1f1-sizeof(struct setup_header)];
	__u32 edd_mbr_sig_buffer[EDD_MBR_SIG_MAX];	/* 0x290 */
	struct boot_e820_entry e820_table[E820_MAX_ENTRIES_ZEROPAGE]; /* 0x2d0 */
	__u8  _pad8[48];				/* 0xcd0 */
	struct edd_info eddbuf[EDDMAXNR];		/* 0xd00 */
	__u8  _pad9[276];				/* 0xeec */
} __attribute__((packed));

/**
 * enum x86_hardware_subarch - x86 hardware subarchitecture
 *
 * The x86 hardware_subarch and hardware_subarch_data were added as of the x86
 * boot protocol 2.07 to help distinguish and support custom x86 boot
 * sequences. This enum represents accepted values for the x86
 * hardware_subarch.  Custom x86 boot sequences (not X86_SUBARCH_PC) do not
 * have or simply *cannot* make use of natural stubs like BIOS or EFI, the
 * hardware_subarch can be used on the Linux entry path to revector to a
 * subarchitecture stub when needed. This subarchitecture stub can be used to
 * set up Linux boot parameters or for special care to account for nonstandard
 * handling of page tables.
 *
 * These enums should only ever be used by x86 code, and the code that uses
 * it should be well contained and compartamentalized.
 *
 * KVM and Xen HVM do not have a subarch as these are expected to follow
 * standard x86 boot entries. If there is a genuine need for "hypervisor" type
 * that should be considered separately in the future. Future guest types
 * should seriously consider working with standard x86 boot stubs such as
 * the BIOS or EFI boot stubs.
 *
 * WARNING: this enum is only used for legacy hacks, for platform features that
 *	    are not easily enumerated or discoverable. You should not ever use
 *	    this for new features.
 *
 * @X86_SUBARCH_PC: Should be used if the hardware is enumerable using standard
 *	PC mechanisms (PCI, ACPI) and doesn't need a special boot flow.
 * @X86_SUBARCH_LGUEST: Used for x86 hypervisor demo, lguest, deprecated
 * @X86_SUBARCH_XEN: Used for Xen guest types which follow the PV boot path,
 * 	which start at asm startup_xen() entry point and later jump to the C
 * 	xen_start_kernel() entry point. Both domU and dom0 type of guests are
 * 	currently supportd through this PV boot path.
 * @X86_SUBARCH_INTEL_MID: Used for Intel MID (Mobile Internet Device) platform
 *	systems which do not have the PCI legacy interfaces.
 * @X86_SUBARCH_CE4100: Used for Intel CE media processor (CE4100) SoC for
 * 	for settop boxes and media devices, the use of a subarch for CE4100
 * 	is more of a hack...
 */
enum x86_hardware_subarch {
	X86_SUBARCH_PC = 0,
	X86_SUBARCH_LGUEST,
	X86_SUBARCH_XEN,
	X86_SUBARCH_INTEL_MID,
	X86_SUBARCH_CE4100,
	X86_NR_SUBARCHS,
};

#endif
