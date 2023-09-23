/*
 * x86 helper functions
 */
#ifndef X86_H
#define X86_H

static inline uint8_t mmio_read_8(uint64_t addr)
{
  return *(uint8_t *) addr;
}

static inline uint16_t mmio_read_16(uint64_t addr)
{
  return *(uint16_t *) addr;
}

static inline uint32_t mmio_read_32(uint64_t addr)
{
  return *(uint32_t *) addr;
}

static inline uint64_t mmio_read_64(uint64_t addr)
{
  return *(uint64_t *) addr;
}

static inline void mmio_write_8(uint64_t addr, uint8_t value)
{
  *(uint8_t *) addr = value;
}

static inline void mmio_write_16(uint64_t addr, uint16_t value)
{
  *(uint16_t *) addr = value;
}

static inline void mmio_write_32(uint64_t addr, uint32_t value)
{
  *(uint32_t *) addr = value;
}

static inline void mmio_write_64(uint64_t addr, uint64_t value)
{
  *(uint64_t *) addr = value;
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t r;
	asm volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t r;
	asm volatile ("inw %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static inline uint32_t inl(uint16_t port)
{
	uint32_t r;
	asm volatile ("inl %1, %0" : "=a"(r) : "Nd"(port));
	return r;
}

static inline void outb(uint16_t port, uint8_t val)
{
	asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static void outl(uint16_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint64_t rdmsr(uint64_t msr)
{
	uint32_t low, high;
	asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
	return ((uint64_t) high << 32) | low;
}

static inline void wrmsr(uint64_t msr, uint64_t value)
{
	uint32_t low = value;
	uint32_t high = value >> 32;

	asm volatile ("wrmsr" :: "c" (msr), "a" (low), "d" (high));
}

static inline uint64_t read_cr0(void)
{
	uint64_t result;

	asm volatile ("movq %%cr0, %0" : "=g" (result));
	return result;
}

static inline uint64_t read_cr3(void)
{
	uint64_t result;

	asm volatile ("movq %%cr3, %0" : "=g" (result));
	return result;
}

static inline uint64_t read_cr4(void)
{
	uint64_t result;

	asm volatile ("movq %%cr4, %0" : "=g" (result));
	return result;
}

#endif
