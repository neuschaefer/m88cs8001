/* SPDX-License-Identifier: MIT */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(x) (1ULL << (x))
#define min(a,b) (((a) < (b))? (a) : (b))

/* MMIO accessors */

static uint8_t  read8(unsigned long addr)  { return *(volatile uint8_t *)addr; }
static uint16_t read16(unsigned long addr) { return *(volatile uint16_t *)addr; }
static uint32_t read32(unsigned long addr) { return *(volatile uint32_t *)addr; }

static void write8(unsigned long addr, uint8_t value)   { *(volatile uint8_t *)addr = value; }
static void write16(unsigned long addr, uint16_t value) { *(volatile uint16_t *)addr = value; }
static void write32(unsigned long addr, uint32_t value) { *(volatile uint32_t *)addr = value; }


/* UART driver */

#define UART_BASE 0xbf540000

static int uart_can_tx(void)
{
	return !(read16(UART_BASE + 0x10) & 0x40);
}

static void uart_tx(char ch)
{
	while (!uart_can_tx())
		;
	write16(UART_BASE + 0x100, ch);
}


/* Console I/O functions */

/* Print one character. LF is converted to CRLF. */
static int putchar(int c)
{
	if (c == '\n')
		uart_tx('\r');
	uart_tx(c);
	return c;
}

/* Print a string. */
static void putstr(const char *s)
{
	for (const char *p = s; *p; p++)
		putchar(*p);
}

/* Print a line. CRLF is added at the end. */
static int puts(const char *s)
{
	putstr(s);
	putchar('\n');
	return 0;
}

/* Print a 8-bit number in hex. */
static void put_hex8(uint8_t x)
{
	static const char hex[16] = "0123456789abcdef";

	putchar(hex[x >> 4]);
	putchar(hex[x & 15]);
}

/* Print a 16-bit number in hex. */
static void put_hex16(uint16_t x)
{
	put_hex8(x >> 8);
	put_hex8(x & 255);
}

/* Print a 32-bit number in hex. */
static void put_hex32(uint32_t x)
{
	put_hex16(x >> 16);
	put_hex16(x & 65535);
}


static void test_uart_rx(void)
{
	int reg, old = -1;

	while (true) {
		reg = read16(UART_BASE + 0x14);
		if (reg != old) {
			put_hex16(reg);
			putchar('\n');
			old = reg;
		}
	}
}

void main(void)
{
	puts("TEST PROGRAM");
	test_uart_rx();
}
