#ifndef BOOTPARAM_H
#define BOOTPARAM_H

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

#define SETUP_NONE			0
#define SETUP_E820_EXT			1
#define SETUP_DTB			2
#define SETUP_PCI			3
#define SETUP_EFI			4
#define SETUP_APPLE_PROPERTIES		5
#define SETUP_JAILHOUSE			6

#define SETUP_INDIRECT			(1<<31)

#define SETUP_TYPE_MAX			(SETUP_INDIRECT | SETUP_JAILHOUSE)

#define RAMDISK_IMAGE_START_MASK	0x07FF
#define RAMDISK_PROMPT_FLAG		0x8000
#define RAMDISK_LOAD_FLAG		0x4000

#define LOADED_HIGH	(1<<0)
#define KASLR_FLAG	(1<<1)
#define QUIET_FLAG	(1<<5)
#define KEEP_SEGMENTS	(1<<6)
#define CAN_USE_HEAP	(1<<7)

#define XLF_KERNEL_64			(1<<0)
#define XLF_CAN_BE_LOADED_ABOVE_4G	(1<<1)
#define XLF_EFI_HANDOVER_32		(1<<2)
#define XLF_EFI_HANDOVER_64		(1<<3)
#define XLF_EFI_KEXEC			(1<<4)
#define XLF_5LEVEL			(1<<5)
#define XLF_5LEVEL_ENABLED		(1<<6)

struct setup_data {
	__u64 next;
	__u32 type;
	__u32 len;
	__u8 data[0];
};

struct setup_indirect {
	__u32 type;
	__u32 reserved;
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

struct olpc_ofw_header {
	__u32 ofw_magic;
	__u32 ofw_version;
	__u32 cif_handler;
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

#define E820_MAX_ENTRIES_ZEROPAGE 128

enum {
	E820_USABLE		= 1,
	E820_RESERVED		= 2,
	E820_ACPI_RECLAIM	= 3,
	E820_APCI_NVS		= 4,
	E820_UNUSABLE		= 5,
};

struct boot_e820_entry {
	__u64 addr;
	__u64 size;
	__u32 type;
} __attribute__((packed));

#define JAILHOUSE_SETUP_REQUIRED_VERSION	1

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
	__u8  orig_x;
	__u8  orig_y;
	__u16 ext_mem_k;
	__u16 orig_video_page;
	__u8  orig_video_mode;
	__u8  orig_video_cols;
	__u8  flags;
	__u8  unused2;
	__u16 orig_video_ega_bx;
	__u16 unused3;
	__u8  orig_video_lines;
	__u8  orig_video_isVGA;
	__u16 orig_video_points;

	__u16 lfb_width;
	__u16 lfb_height;
	__u16 lfb_depth;
	__u32 lfb_base;
	__u32 lfb_size;
	__u16 cl_magic, cl_offset;
	__u16 lfb_linelength;
	__u8  red_size;
	__u8  red_pos;
	__u8  green_size;
	__u8  green_pos;
	__u8  blue_size;
	__u8  blue_pos;
	__u8  rsvd_size;
	__u8  rsvd_pos;
	__u16 vesapm_seg;
	__u16 vesapm_off;
	__u16 pages;
	__u16 vesa_attributes;
	__u32 capabilities;
	__u32 ext_lfb_base;
	__u8  _reserved[2];
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


#define EDD_MBR_SIG_MAX 16
#define EDDMAXNR 6

struct edd_device_params {
	__u16 length;
	__u16 info_flags;
	__u32 num_default_cylinders;
	__u32 num_default_heads;
	__u32 sectors_per_track;
	__u64 number_of_sectors;
	__u16 bytes_per_sector;
	__u32 dpte_ptr;
	__u16 key;
	__u8 device_path_info_length;
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

struct boot_params {
	struct screen_info screen_info;
	struct apm_bios_info apm_bios_info;
	__u8  _pad2[4];
	__u64  tboot_addr;
	struct ist_info ist_info;
	__u64 acpi_rsdp_addr;
	__u8  _pad3[8];
	__u8  hd0_info[16];
	__u8  hd1_info[16];
	struct sys_desc_table sys_desc_table;
	struct olpc_ofw_header olpc_ofw_header;
	__u32 ext_ramdisk_image;
	__u32 ext_ramdisk_size;
	__u32 ext_cmd_line_ptr;
	__u8  _pad4[116];
	struct edid_info edid_info;
	struct efi_info efi_info;
	__u32 alt_mem_k;
	__u32 scratch;
	__u8  e820_entries;
	__u8  eddbuf_entries;
	__u8  edd_mbr_sig_buf_entries;
	__u8  kbd_status;
	__u8  secure_boot;
	__u8  _pad5[2];
	__u8  sentinel;
	__u8  _pad6[1];
	struct setup_header hdr;
	__u8  _pad7[0x290-0x1f1-sizeof(struct setup_header)];
	__u32 edd_mbr_sig_buffer[EDD_MBR_SIG_MAX];
	struct boot_e820_entry e820_table[E820_MAX_ENTRIES_ZEROPAGE];
	__u8  _pad8[48];
	struct edd_info eddbuf[EDDMAXNR];
	__u8  _pad9[276];
} __attribute__((packed));

enum x86_hardware_subarch {
	X86_SUBARCH_PC = 0,
	X86_SUBARCH_LGUEST,
	X86_SUBARCH_XEN,
	X86_SUBARCH_INTEL_MID,
	X86_SUBARCH_CE4100,
	X86_NR_SUBARCHS,
};

#endif
