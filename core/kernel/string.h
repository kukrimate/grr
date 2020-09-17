#ifndef STRING_H
#define STRING_H

/* Calculate the size of an array */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

/*
 * Set n bytes of memory starting from s to c
 */
void *
__attribute__((sysv_abi))
memset(void *s, int c, size_t n);

/*
 * Set n bytes of memory starting from s to 0
 */
#define bzero(s, n) memset(s, 0, n)

/*
 * Copy n bytes from src to dest, handling overlaps
 */
void *
memmove(void *dest, const void *src, size_t n);

/*
 * Copy n bytes from src to dest, overlapping regions cause undefined behaviour
 */
void *
memcpy(void *dest, const void *src, size_t n);

/*
 * Compare two n byte memory regions
 */
int
memcmp(const void *s1, const void *s2, size_t n);

/*
 * Compute the length (excluding NUL) of an ASCII string
 */
size_t
strlen(const char *s);

#endif
