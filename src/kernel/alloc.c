/*
 * Memory allocator
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include <include/handover.h>
#include <kernel/alloc.h>
#include <kernel/uart.h>

/*
 * Memory blocks
 */
struct memblock {
	uint8_t	*addr;
	size_t	pages;
};

static size_t alloc_blockcnt = 0;
static struct memblock *alloc_blockmap = NULL;

#define GET_BIT(x, b) ((x)[(b) / 8] & (1 << ((b) % 8)))
#define SET_BIT(x, b) ((x)[(b) / 8] |= (1 << ((b) % 8)))
#define CLR_BIT(x, b) ((x)[(b) / 8] &= ~(1 << ((b) % 8)))

void
alloc_init(struct grr_handover *handover)
{
	size_t block_idx, page_idx;
	uint64_t bitmap_pages;

	/*
	 * Intialize the page allocator
	 */
	uart_print("Hypervisor memory:\n");
	for (block_idx = 0; block_idx < handover->hmem_entries; ++block_idx) {
		uart_print("\t%p-%p\n", handover->hmem[block_idx].addr,
			handover->hmem[block_idx].addr +
			handover->hmem[block_idx].size - 1);

		/* The highest page of the first block becomes the blockmap
		   FIXME: this breaks in a spectecular fashion
		   	if we get a 1 page long memory block
		   FIXME2: the blockmap is always one page, so if we get >256
		   	memory blocks, it will cause breakage again */
		if (!alloc_blockmap) {
			alloc_blockmap = (void *) (handover->hmem[block_idx].addr
				+ handover->hmem[block_idx].size - PAGE_SIZE);
			memset(alloc_blockmap, 0, PAGE_SIZE);
		}

		/* Each block starts with a bitmap for said block */
		++alloc_blockcnt;
		alloc_blockmap[block_idx].addr = (void *) handover->hmem[block_idx].addr;
		alloc_blockmap[block_idx].pages = PAGE_COUNT(handover->hmem[block_idx].size);
		/* Number of pages needed for this block's bitmap */
		bitmap_pages = PAGE_COUNT(handover->hmem[block_idx].size / PAGE_SIZE / 8);
		memset(alloc_blockmap[block_idx].addr, 0, PAGE_SIZE * bitmap_pages);

		/* Mark pages as free or used in the block */
		for (page_idx = 0; page_idx < alloc_blockmap[block_idx].pages; ++page_idx) {
			/* Mark the page used if it is the blockmap or
			   part of this blocks bitmap */
			if ((void *) handover->hmem[block_idx].addr
					+ PAGE_SIZE * page_idx == alloc_blockmap
					|| page_idx < bitmap_pages) {
				SET_BIT(alloc_blockmap[block_idx].addr, page_idx);
			}
		}
	}
}

void *
alloc_pages(size_t count, uint64_t maxaddr)
{
	size_t block_idx, page_idx;
	size_t freebase, freecnt;

	if (!count) /* Can't allocate 0 pages */
		goto fail;

	/* Scan for a free region big enough */
	block_idx = alloc_blockcnt - 1;
	do {
		freebase = freecnt = 0;
		for (page_idx = 0; page_idx < alloc_blockmap[block_idx].pages; ++page_idx) {
			if (!GET_BIT(alloc_blockmap[block_idx].addr, page_idx)) {
				if (!freebase)
					freebase = page_idx;
				++freecnt;
			} else {
				if (freecnt >= count && (!maxaddr || (uint64_t) alloc_blockmap[block_idx].addr
						+ PAGE_SIZE * freebase <= maxaddr))
					goto alloc;
				else
					freebase = freecnt = 0;
			}
		}
		if (freecnt >= count && (!maxaddr || (uint64_t) alloc_blockmap[block_idx].addr
				+ PAGE_SIZE * freebase <= maxaddr))
			goto alloc;
	} while (block_idx--);
fail:
	uart_print("OUT OF MEMORY!!!\n");
	for (;;)
		;
	return NULL;
alloc:
	while (count--)
		SET_BIT(alloc_blockmap[block_idx].addr, freebase + count);
	return alloc_blockmap[block_idx].addr + PAGE_SIZE * freebase;
}

void
free_pages(void *addr, size_t count)
{
	size_t block_idx, start;

	if (!addr || !count) /* Can't free non-existent pages */
		return;

	for (block_idx = 0; block_idx < alloc_blockcnt; ++block_idx)
		if ((uint64_t) addr >= (uint64_t) alloc_blockmap[block_idx].addr
				&& (uint64_t) addr + PAGE_SIZE * count <=
				(uint64_t) alloc_blockmap[block_idx].addr
				+ PAGE_SIZE * alloc_blockmap[block_idx].pages) {
			start = PAGE_COUNT((uint64_t) addr -
				(uint64_t) alloc_blockmap[block_idx].addr);
			while (count--)
				CLR_BIT(alloc_blockmap[block_idx].addr, start + count);
		}
}
