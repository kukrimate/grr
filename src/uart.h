#ifndef UART_H
#define UART_H

/*
 * Use COM1 at I/O 0x3f8
 */
#define UART_ADDR 0x3f8

/*
 * Set baudrate to 115.2k
 */
#define UART_BAUD 115200

/*
 * Use an ANSI terminal
 */
#define AURT_ANSI

/* Intialize the UART to known settings */
void
uart_setup(void);

/* Write a byte to the UART */
void
uart_write(uint8_t data);

/* Read a byte from the UART */
uint8_t
uart_read(void);

/* Printf-like function over the UART */
void
uart_print(const char *fmt, ...);

#endif
