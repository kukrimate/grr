/*
 * Heap allocator
 */

#include <stddef.h>

/*
 * Heap is defined in helper.S
 */
void vmm_heap();
void vmm_heap_end();

/*
 * Current heap pointer
 */
void *heap_cur = vmm_heap;

void *
alloc_pages(size_t count)
{
	size_t size;
	void *buffer;

	size = count * 4096;
	if (heap_cur + size <= (void *) vmm_heap_end) {
		buffer = heap_cur;
		heap_cur += size;
	} else {
		buffer = NULL;
	}
	return buffer;
}
