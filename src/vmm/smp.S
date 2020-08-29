/*
 * SMP initialization trampoline
 */

.section .data

/*
 * SMP init GDT
 */
smp_gdt:
.quad	0
.quad	0x00cf9a000000ffff	/* CS32 */
.quad	0x00cf92000000ffff	/* DS32 */
.quad	0x00209a0000000000	/* CS64 */
.quad	0x0000920000000000	/* DS64 */
smp_gdt_end:

.section .text

/*
 * 16-bit SMP init trampoline
 * NOTE: This has to be position independent
 *   because the C SMP init code will memcpy <1MiB
 */
.global smp_init16
smp_init16:
.code16

/* Setup %ds and %ss */
movw %cs, %ax
movw %ax, %ds
movw %ax, %ss
/* Load stack (relative to IP=0, which is this function's start) */
movw $smp_init16_end - smp_init16, %sp

/* Load GDT */
pushl $smp_gdt
pushw $smp_gdt_end - smp_gdt
movw %sp, %si
lgdtw (%si)
addw $6, %sp

/* Enable protected mode */
movl %cr0, %eax
orl $1, %eax
movl %eax, %cr0

/* Jump into protected mode */
ljmpl $0x08, $smp_init32

/* NOTE: be careful, we only have 16 bytes of stack space */
.skip 16
.global smp_init16_end
smp_init16_end:

/*
 * 32-bit SMP init
 */
smp_init32:
.code32
movl $0x10, %eax
movl %eax, %ds
movl %eax, %es
movl %eax, %ss
movl %eax, %fs
movl %eax, %gs

1:
jmp 1b

/*
 * 64-bit SMP init
 */
smp_init64:
.code64
