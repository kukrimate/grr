#ifndef KERNEL_H
#define KERNEL_H

/*
 * Kernel lowmem allocator
 */
void *
kernel_lowmem_alloc(size_t pages);

/*
 * Kernel root page table, used by the SMP setup code
 */
extern
uint64_t *kernel_pml4;

/*
 * Big kernel lock, its terrible, but also very easy
 */
void
kernel_bkl_acquire(void);

void
kernel_bkl_release(void);

#endif
