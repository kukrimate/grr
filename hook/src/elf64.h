#ifndef ELF64_H
#define ELF64_H

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct elf64_hdr {
	u8	e_ident[16];
	u16	e_type;
	u16	e_machine;
	u32	e_version;
	u64	e_entry;
	u64	e_phoff;
	u64	e_shoff;
	u32	e_flags;
	u16	e_ehsize;
	u16	e_phentsize;
	u16	e_phnum;
	u16	e_shentsize;
	u16	e_shnum;
	u16	e_shstrndx;
};

#define PT_NULL 0
#define PT_LOAD 1

struct elf64_phdr {
	u32	p_type;
	u32	p_flags;
	u64	p_offset;
	u64	p_vaddr;
	u64	p_paddr;
	u64	p_filesz;
	u64	p_memsz;
	u64	p_align;
};

#endif
