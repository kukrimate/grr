/*
 * Memory allocator
 */

#include <stddef.h>
#include <stdint.h>
#include <khelper.h>
#include <include/handover.h>
#include <kernel/alloc.h>
#include <kernel/kernel.h>
#include <kernel/uart.h>

/*
 * Memory block
 */
struct memblock {
	/* Address of the first page */
	uint8_t	*addr;
	/* # of pages */
	size_t	pages;
};

/*
 * List of memory blocks
 */
static struct memblock *blocks;

#define GET_BIT(x, b) ((x)[(b) / 8] & (1 << ((b) % 8)))
#define SET_BIT(x, b) ((x)[(b) / 8] |= (1 << ((b) % 8)))
#define CLR_BIT(x, b) ((x)[(b) / 8] &= ~(1 << ((b) % 8)))

void
alloc_init(struct grr_handover *handover)
{
	struct hmem_entry *hmem, *hmem_end, tmp;
	_Bool dirty;
	struct memblock *block;
	size_t bitmap_pages, i_pg;

	uart_print("Setting up page allocator\n");

	/* End of the memory map */
	hmem_end = handover->hmem + handover->hmem_entries;

	/* Reverse sort the memory map, as we always allocate from top-down */
	do {
		dirty = 0;
		for (hmem = handover->hmem + 1; hmem < hmem_end; ++hmem) {
			if ((hmem - 1)->addr < hmem->addr) {
				tmp.addr = hmem->addr;
				tmp.size = hmem->size;
				hmem->addr = (hmem - 1)->addr;
				hmem->size = (hmem - 1)->size;
				(hmem - 1)->addr = tmp.addr;
				(hmem - 1)->size = tmp.size;
				dirty = 1;
			}
		}
	} while (dirty);

	/* Allocator memory block */
	block = NULL;

	for (hmem = handover->hmem; hmem < hmem_end; ++hmem) {
		uart_print("Usable memory at: [%p-%p]\n", hmem->addr,
			hmem->addr + hmem->size -1);

		if (!block) {
			/* Allocate the block list from the first block */
			block = (void *) hmem->addr + hmem->size - PAGE_SIZE;
			/* Store a pointer to the block list in a global */
			blocks = block;
		}

		block->addr = (void *) hmem->addr;
		block->pages = PAGE_COUNT(hmem->size);

		/* Number of pages needed for the block's bitmap */
		bitmap_pages = PAGE_COUNT(block->pages / 8);

		/* Make sure there is no leftover data in the bitmap */
		bzero(block->addr, PAGE_SIZE * bitmap_pages);

		/* Mark used pages as such in the bitmap */
		for (i_pg = 0; i_pg < block->pages; ++i_pg)
			/* A page can be used by the block list or the bitmap */
			if (block->addr + i_pg * PAGE_SIZE == (void *) blocks
					|| i_pg < bitmap_pages)
				SET_BIT(block->addr, i_pg);

		/* Move pointer to the next memory block */
		++block;
	}

	/* Terminate the block list */
	block->addr = NULL;
	block->pages = 0;

	uart_print("Page alloctor setup complete\n");
}

/*
 * Only one thread can write to the block list at a time
 */
static int blocklist_wr_lock;

void *
alloc_pages(size_t cnt, void *below)
{
	struct memblock *block;
	size_t freecnt, i_pg;

	if (!cnt)
		goto fail;

	for (block = blocks; block->pages; ++block) {
		freecnt = 0;
		i_pg = block->pages;

		while (i_pg--)
			if (!GET_BIT(block->addr, i_pg) && ((void *) block->addr
					+ i_pg * PAGE_SIZE < below || !below)) {
				if (++freecnt >= cnt)
					goto allocate;
			} else {
				freecnt = 0;
			}
	}

fail:
	/* Other code relies on this never returning NULL */
	uart_print("OUT OF MEMORY!\n");
	for (;;)
		;
	return NULL;

allocate:
	spinlock_lock(blocklist_wr_lock);
	while (cnt--)
		SET_BIT(block->addr, i_pg + cnt);
	spinlock_unlock(blocklist_wr_lock);
	return block->addr + i_pg * PAGE_SIZE;
}

void
free_pages(void *addr, size_t cnt)
{
	struct memblock *block;
	size_t i_pg;

	/* We can't free non-existent pages */
	if (!addr || !cnt)
		return;

	/* Look for the block the memory is allocated from */
	for (block = blocks; block->pages; ++block)
		if (addr >= (void *) block->addr &&
				addr < (void *) block->addr
				+ block->pages * PAGE_SIZE) {
			/* Free the pages */
			i_pg = (addr - (void *) block->addr) / PAGE_SIZE;
			spinlock_lock(blocklist_wr_lock);
			while (cnt--)
				CLR_BIT(block->addr, i_pg + cnt);
			spinlock_unlock(blocklist_wr_lock);
			break;
		}
}
