#ifndef ALLOC_H
#define ALLOC_H

/*
 * Physical page size
 */
#define PAGE_SIZE 0x1000

/*
 * Number of pages needed to fit an object
 */
#define PAGE_COUNT(x) ((x + PAGE_SIZE - 1) / PAGE_SIZE)

/*
 * NOTE: most users don't need this, so forward declare it here
 */
struct grr_handover;

/*
 * Intialize the page allocator
 */
void
alloc_init(struct grr_handover *handover);

/*
 * Allocate count pages
 * If below is non-zero, only pages below that address will be returned
 */
void *
alloc_pages(size_t cnt, void *below);

/*
 * Free count pages starting at addr
 */
void
free_pages(void *addr, size_t cnt);

#endif
