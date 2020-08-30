/*
 * x86 helper functions
 */
#ifndef X86_H
#define X86_H

static
uint8_t
inb(uint16_t port)
{
	uint8_t r;
	asm volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static
uint16_t
inw(uint16_t port)
{
	uint16_t r;
	asm volatile ("inw %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static
uint32_t
inl(uint16_t port)
{
	uint32_t r;
	asm volatile ("inl %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static
void
outb(uint16_t port, uint8_t val)
{
	asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static
void
outw(uint16_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static
void
outl(uint16_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static
uint64_t
rdmsr(uint64_t msr)
{
	uint32_t low, high;
	asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
	return ((uint64_t) high << 32) | low;
}

static
void
wrmsr(uint64_t msr, uint64_t value)
{
	uint32_t low = value;
	uint32_t high = value >> 32;

	asm volatile ("wrmsr" :: "c" (msr), "a" (low), "d" (high));
}

static
uint64_t
read_cr0(void)
{
	uint64_t result;

	asm volatile ("movq %%cr0, %0" : "=g" (result));
	return result;
}

static
uint64_t
read_cr3(void)
{
	uint64_t result;

	asm volatile ("movq %%cr3, %0" : "=g" (result));
	return result;
}

static
uint64_t
read_cr4(void)
{
	uint64_t result;

	asm volatile ("movq %%cr4, %0" : "=g" (result));
	return result;
}

#endif
