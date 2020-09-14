#ifndef VMM_H
#define VMM_H

/*
 * Virtual machine control block
 */
struct vmcb {
	/*
	 * Intercept vectors
	 */
	u16 cr_read;		/* Offset: 0x000 */
	u16 cr_write;

	u16 dr_read;		/* Offset: 0x004 */
	u16 dr_write;

	u32 exception;		/* Offset: 0x008 */

	u32 intr  	: 1;	/* Offset: 0x00c */
	u32 nmi   	: 1;
	u32 smi		: 1;
	u32 init	: 1;
	u32 vintr	: 1;
	u32 cr0_write	: 1;	/* CR0.TS and CR0.MP are ignored */
	u32 idtr_read	: 1;
	u32 gdtr_read	: 1;
	u32 ldtr_read	: 1;
	u32 tr_read	: 1;
	u32 idtr_write	: 1;
	u32 gdtr_write	: 1;
	u32 ldtr_write	: 1;
	u32 tr_write	: 1;
	u32 rdtsc	: 1;
	u32 rdpmc	: 1;
	u32 pushf	: 1;
	u32 popf	: 1;
	u32 cpuid	: 1;
	u32 rsm		: 1;
	u32 iret	: 1;
	u32 intn	: 1;
	u32 invd	: 1;
	u32 pause	: 1;
	u32 hlt		: 1;
	u32 invlpg	: 1;
	u32 invlpga	: 1;
	u32 io_prot	: 1;	/* selected ports */
	u32 msr_prot	: 1;	/* selected MSRs */
	u32 task_switch	: 1;
	u32 ferr_freeze	: 1;
	u32 shutdown	: 1;

	u16 vmrun	: 1;	/* Offset: 0x10 */
	u16 vmmcall	: 1;
	u16 vmload	: 1;
	u16 vmsave	: 1;
	u16 stgi	: 1;
	u16 clgi	: 1;
	u16 skinit	: 1;
	u16 rdtscp	: 1;
	u16 icebp	: 1;
	u16 wbindv	: 1;	/* wbindv and wbnoinvd */
	u16 monitor	: 1;	/* monitor and monitorx */
	u16 mwait	: 1; 	/* mwait and mwaitx */
	u16 mwait_armed	: 1;	/* only when monitor hardware is armed */
	u16 xsetbv	: 1;
	u16 rdpru	: 1;
	u16 efer_write	: 1;
	u16 cr_wr_post	: 1;	/* after the write finished */

	u32 invlpgb	: 1;	/* Offset: 0x14 */
	u32 invlpgb_il  : 1;	/* only illegally specified ones */
	u32 pcid	: 1;
	u32 mcommit	: 1;
	u32 tlbsync	: 1;
	u32 reserved1	: 27;
	/* End of intercept vectors */

	u8  reserved2[0x24];	/* Offset: 0x18 */

	u16 pause_treshold;	/* Offset: 0x3c */
	u16 pause_count;	/* Offset: 0x3e */
	u64 iopm_base_pa;	/* Offset: 0x40 */
	u64 msrpm_base_pa;	/* Offset: 0x48 */
	u64 tsc_offset;		/* Offset: 0x50 */

	u64 guest_asid	: 32;	/* Offset: 0x58 */
	u64 tlb_control	: 8;
	u64 reserved3	: 24;

	u64 v_tpr	: 8; 	/* Offset: 0x60 */
	u64 v_irq	: 1;
	u64 vgif	: 1;
	u64 reserved4	: 6;
	u64 v_intr_prio	: 4;
	u64 v_ign_tpr	: 1;
	u64 reserved5	: 3;
	u64 v_intr_mask	: 1;
	u64 amd_v_gif	: 1;
	u64 reserved6	: 5;
	u64 avic_en	: 1;
	u64 v_intr_vec	: 8;
	u64 reserved7	: 24;

	u64 g_int_shdw	: 1;	/* Offset: 0x68 */
	u64 g_int_mask	: 1;
	u64 reserved8	: 62;

	u64 exitcode;		/* Offset: 0x70 */
	u64 exitinfo1;		/* Offset: 0x78 */
	u64 exitinfo2;		/* Offset: 0x80 */
	u64 exitintinfo;	/* Offset: 0x88 */

	u64 np_en	: 1;	/* Offset: 0x90 */
	u64 sev_en	: 1;
	u64 stat_sev_en : 1;
	u64 g_exec_trap	: 1;
	u64 reserved9	: 1;
	u64 vte_en	: 1;
	u64 reserved10	: 58;

	u64 avic_apic_bar;	/* Offset: 0x98 */
	u64 ghcb_gpa;		/* Offset: 0xa0 */
	u64 eventinj;		/* Offset: 0xa8 */
	u64 n_cr3;		/* Offset: 0xb0 */

	u64 lbr_vir_en	: 1;	/* Offset: 0xb8 */
	u64 vmsv_vir_en : 1;	/* virtualize vmsave/vmload */
	u64 reserved11	: 62;

	u32 vmcb_clean_bits;	/* Offset: 0xc0 */
	u32 reserved12;

	u64 nrip;		/* Offset: 0xc8 */

	u64 nb_fetched	: 8;	/* Offset: 0xd0 */
	u64 g_insn_bl	: 56;
	u64 g_insn_bh;

	u64 avic_apic_pg;	/* Offset: 0xe0 */

	u64 reserved13;		/* Offset: 0xe8 */

	u64 avic_logic_tab;	/* Offset: 0xf0 */

	u64 avic_phs_mx	: 8;	/* Offset: 0xf8 */
	u64 avic_phs_tb	: 56;

	u64 reserved14;		/* Offset: 0x100 */

	u64 vmsa_ptr;		/* Offset: 0x108 */

	u8 reserved15[752];	/* Pad to the save state */

	/*
	 * Save state area
	 */

	u16 es_selector;
	u16 es_attrib;
	u32 es_limit;
	u64 es_base;

	u16 cs_selector;
	u16 cs_attrib;
	u32 cs_limit;
	u64 cs_base;

	u16 ss_selector;
	u16 ss_attrib;
	u32 ss_limit;
	u64 ss_base;

	u16 ds_selector;
	u16 ds_attrib;
	u32 ds_limit;
	u64 ds_base;

	u16 fs_selector;
	u16 fs_attrib;
	u32 fs_limit;
	u64 fs_base;

	u16 gs_selector;
	u16 gs_attrib;
	u32 gs_limit;
	u64 gs_base;

	u32 reserved16;
	u32 gdtr_limit;
	u64 gdtr_base;

	u32 reserved17;
	u32 ldtr_limit;
	u64 ldtr_base;

	u32 reserved18;
	u32 idtr_limit;
	u64 idtr_base;

	u32 reserved19;
	u32 tr_limit;
	u64 tr_base;

	u8  reserved20[43];
	u8  cpl;
	u32 reserved21;
	u64 efer;
	u8  reserved22[112];
	u64 cr4;
	u64 cr3;
	u64 cr0;
	u64 dr7;
	u64 dr6;
	u64 rflags;
	u64 rip;
	u8  reserved23[88];
	u64 rsp;
	u8  reserved24[24];
	u64 rax;
	u64 star;
	u64 lstar;
	u64 cstar;
	u64 sfmask;
	u64 kernel_gs_base;
	u64 sysenter_cs;
	u64 systenter_esp;
	u64 sysenter_eip;
	u64 cr2;
	u8  reserved25[32];
	u64 g_pat;
	u64 dbgctl;
	u64 br_from;
	u64 br_to;
	u64 lastexcpfrom;
	u64 lastexcpto;

	u8  reserved26[2408];		/* Pad to fill a 4K page */
} __attribute__((packed));

/*
 * General purpose registers
 */
struct gprs {
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
} __attribute__((packed));

struct vmm_cpu {
	/*
	 * VMCB for the CPU
	 */
	struct vmcb vmcb;
	/*
	 * Guest register state
	 */
	struct gprs gprs;
	/*
	 * LAPIC page table
	 */
	uint64_t *lapic_pt;
	/*
	 * LAPIC emulator page
	 */
	void *lapic_emu;
	/*
	 * LAPIC emulator register
	 */
	size_t lapic_reg;
};

struct vmm_cpu *
vmm_setup_bsp(struct grr_handover *handover);

struct vmm_cpu *
vmm_setup_ap(void);

void
vmm_execute(struct vmm_cpu *ctx);

#endif
