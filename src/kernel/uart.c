/*
 * Polled UART driver
 */

#include <stdint.h>
#include <stdarg.h>
#include <include/x86.h>
#include <kernel/acpi.h>
#include <kernel/kernel.h>
#include <kernel/uart.h>

/* UART registers */
enum {
	UART_BUF = 0,	/* Receive/Transmit buffer (DLAB=0, R/W) */
	UART_DLL = 0,	/* Divisor latch LSB (DLAB=1, R/W) */
	UART_IER = 1,	/* Interrupt enable (DLAB=0, R/W) */
	UART_DLM = 1,	/* Divisor latch MSB (DLAB=1, R/W) */
	UART_IIR = 2,	/* Interrupt indetification (R/O) */
	UART_FCR = 2,	/* FIFO control (W/O) */
	UART_LCR = 3,	/* Line control (R/W) */
	UART_MCR = 4,	/* Modem control (R/W) */
	UART_LSR = 5,	/* Line status (R/O) */
	UART_MSR = 6,	/* Modem status (R/O) */
	UART_SCR = 7,	/* Scratch register (R/W) */
};

void
uart_setup(void)
{
	uint16_t divisor;
	const char *p;

	divisor = 115200 / UART_BAUD;     /* The divisor is MAX_BAUD / BAUD */
	outb(UART_ADDR + UART_LCR, 0x80); /* Set DLAB=1 */

	/* Set baudrate */
	outb(UART_ADDR + UART_DLL, (uint8_t) divisor);
	outb(UART_ADDR + UART_DLM, (uint8_t) (divisor >> 8));

	outb(UART_ADDR + UART_LCR, 3);    /* Use 1N8 mode */
	outb(UART_ADDR + UART_IER, 0);    /* Disable interrupts */
	outb(UART_ADDR + UART_FCR, 0);    /* Disable FIFO */

#ifdef UART_ANSI
	/* Do ANSI reset */
	for (p = "\033[0m"; *p; ++p)
		uart_write(*p);
#endif
}

void
uart_write(uint8_t data)
{
	/* Wait for THR to be empty */
	while (!(inb(UART_ADDR + UART_LSR) & (1 << 5)));
	/* Write data */
	outb(UART_ADDR + UART_BUF, data);
}

uint8_t
uart_read(void)
{
	/* Wait for data avaiable to be set */
	while (!(inb(UART_ADDR + UART_LSR) & 1));
	/* Read data */
	return inb(UART_ADDR + UART_BUF);
}

static
const char *digits = "0123456789abcdef";

#define genprint(U, S) \
static \
void \
uart_print_##U(U num, int base) \
{ \
	char buf[20], *p; \
\
	p = buf + sizeof(buf); \
	*--p = 0; \
\
	do { \
		*--p = digits[num % base]; \
	} while (p >= buf && (num /= base)); \
\
	for (; *p; ++p) \
		uart_write(*p); \
} \
\
static \
void \
uart_print_##S(S num, int base) \
{ \
	if (num < 0) { \
		uart_write('-'); \
		num *= -1; \
	} \
	uart_print_##U(num, base); \
}

genprint(uint32_t, int32_t)
genprint(uint64_t, int64_t)

/*
 * Only one thread can print at a time
 */
static int print_lock;

void
uart_print(const char *fmt, ...)
{
	va_list va;
	_Bool wide;
	const char *p, buf[20];

	spinlock_lock(print_lock);

	/* Print APIC id */
	uart_write('[');
	uart_print_uint32_t(acpi_get_apic_id(), 10);
	uart_write(']');
	uart_write(' ');

	va_start(va, fmt);
	for (; *fmt; ++fmt)
		switch (*fmt) {
		case '%':
			wide = 0;
wide_redo:
			switch (*++fmt) {
			case 'l':
				wide = 1;
				goto wide_redo;
			case 'c':
				uart_write(va_arg(va, int));
				break;
			case 's':
				for (p = va_arg(va, const char *); *p; ++p)
					uart_write(*p);
				break;
			case 'p':
				uart_print_uint64_t(va_arg(va, uint64_t), 16);
				break;
			case 'x':
				if (wide)
					uart_print_uint64_t(va_arg(va, uint64_t), 16);
				else
					uart_print_uint32_t(va_arg(va, uint32_t), 16);
				break;
			case 'd':
				if (wide)
					uart_print_int64_t(va_arg(va, int64_t), 10);
				else
					uart_print_int32_t(va_arg(va, int32_t), 10);
				break;
			case '%':
				uart_write('%');
				break;
			default:
				uart_write('?');
				break;
			}
			break;
		case '\r': /* Ignore CR */
			break;
		case '\n': /* Write CRLF on LF */
			uart_write('\r');
			uart_write('\n');
			break;
		default:
			uart_write(*fmt);
			break;
		}
	va_end(va);

	spinlock_unlock(print_lock);
}
