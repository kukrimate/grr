/*
 * Virtual machine monitor
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>

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

void
vmm_startup(void *linux_entry, void *boot_params)
{
	load_gdt();
	asm volatile (
		"movq %0, %%rax\n"
		"movq %1, %%rsi\n"
		"jmp *%%rax" ::
		"g" (linux_entry),
		"g" (boot_params)
		: "rax", "rsi");
}
