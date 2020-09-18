#ifndef KERNEL_H
#define KERNEL_H

/*
 * Simple spinlock
 */
#define spinlock_lock(x) \
	asm volatile ("1: lock btsl $0, %0; jc 1b" : "=m" (x))
#define spinlock_unlock(x) \
	asm volatile ("lock btrl $0, %0" : "=m" (x))

/*
 * Kernel root page table, used by the SMP setup code
 */
extern
uint64_t *kernel_pml4;

/*
 * Kernel GDT
 */
extern
uint64_t kernel_gdt[];

extern
uint16_t
kernel_gdt_size;

/*
 * Load the kernel GDT and IDT, used by the SMP setup code
 */
void
kernel_cpu_init(void);

#endif
