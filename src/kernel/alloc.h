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
 * Allocate count pages, limiting addresses to maxaddr (or no limit if NULL)
 */
void *
alloc_pages(size_t count, uint64_t maxaddr);

/*
 * Free count pages starting at addr
 */
void
free_pages(void *addr, size_t count);

#endif
