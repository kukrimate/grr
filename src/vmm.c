/*
 * Virtual machine monitor
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include "vmcb.h"
#include "uart.h"
#include "x86.h"

static
uint64_t
gdt[] = {
	0,
	0,
	0x00209A0000000000,	/* __BOOT_CS */
	0x0000920000000000,	/* __BOOT_DS */
};

static
void
load_gdt(void)
{
	struct {
		uint16_t limit;
		uint64_t addr;
	} __attribute__((packed)) gdtr;

	gdtr.limit = sizeof(gdt);
	gdtr.addr = (uint64_t) gdt;

	asm volatile ("lgdt %0\n"
			"pushq $0x10\n"
			"pushq $reload_cs\n"
			"retfq; reload_cs:\n"
			"movl $0x18, %%eax\n"
			"movl %%eax, %%ds\n"
			"movl %%eax, %%es\n"
			"movl %%eax, %%ss\n"
			"movl %%eax, %%fs\n"
			"movl %%eax, %%gs"
			 :: "m" (gdtr) : "rax");
}

/*
 * Put the VMCB in .bss
 */
static
struct vmcb vmcb;

#define MSR_EFER	0xC0000080
# define EFER_SVME	(1 << 12)

void
vmm_startup(void *linux_entry, void *boot_params)
{
	u64 efer;

	uart_setup(); /* Make sure we can print stuff */

	/* Enable SVM */
	uart_print("Enabling SVM...\n", efer);
	efer = rdmsr(MSR_EFER);
	// wrmsr(MSR_EFER, efer | EFER_SVME);

	/* Start the guest */
	uart_print("Starting guest...\n");
	asm volatile (
		"movq %0, %%rax\n"
		"vmrun"
		:: "g" (&vmcb) : "rax");


	// load_gdt();
	// asm volatile (
	// 	"movq %0, %%rax\n"
	// 	"movq %1, %%rsi\n"
	// 	"jmp *%%rax" ::
	// 	"g" (linux_entry),
	// 	"g" (boot_params)
	// 	: "rax", "rsi");
}
