/*
 * Standard string functions
 */
#include <stddef.h>

void *
memset(void *s, int c, size_t n)
{
	char *p;

	for (p = s; n; --n)
		*p++ = c;

	return s;
}

void *
memmove(void *dest, void *src, size_t n)
{
	char *d, *s;

	d = dest;
	s = src;

	if (s < d) {
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	} else {
		while (n--)
			*d++ = *s++;
	}

	return dest;
}

void *
memcpy(void *dest, void *src, size_t n)
{
	char *d, *s;

	d = dest;
	s = src;

	while (n--)
		*d++ = *s++;

	return dest;
}

int
memcmp(const void *s1, const void *s2, size_t n)
{
	const char *p1, *p2;

	if (n) {
		p1 = s1;
		p2 = s2;
		do {
			if (*p1++ != *p2++)
				return *--p1 - *--p2;
		} while (--n);
	}

	return 0;
}
size_t
strlen(const char *s)
{
	const char *p;

	for (p = s; *p; ++p)
		;

	return p - s;
}
