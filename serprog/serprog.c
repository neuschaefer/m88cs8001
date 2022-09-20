/* SPDX-License-Identifier: MIT */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(x) (1ULL << (x))
#define min(a,b) (((a) < (b))? (a) : (b))
#define max(a,b) (((a) > (b))? (a) : (b))
#define KiB (1 << 10)
#define MiB (1 << 20)
#define GiB (1 << 30)

/* MMIO accessors */

static uint8_t  read8(unsigned long addr)  { return *(volatile uint8_t *)addr; }
static uint16_t read16(unsigned long addr) { return *(volatile uint16_t *)addr; }
static uint32_t read32(unsigned long addr) { return *(volatile uint32_t *)addr; }

static void write8(unsigned long addr, uint8_t value)   { *(volatile uint8_t *)addr = value; }
static void write16(unsigned long addr, uint16_t value) { *(volatile uint16_t *)addr = value; }
static void write32(unsigned long addr, uint32_t value) { *(volatile uint32_t *)addr = value; }


/* UART driver */

#define UART_BASE 0xbf540000
#define UART_FIFO_MAX 64

static int uart_tx_level(void)
{
	return read16(UART_BASE + 0x10);
}

static int uart_rx_level(void)
{
	return read16(UART_BASE + 0x14);
}

static void uart_tx(char ch)
{
	while (uart_tx_level() >= UART_FIFO_MAX)
		;
	write16(UART_BASE + 0x100, ch);
}

static char uart_rx(void)
{
	while (uart_rx_level() == 0)
		;
	return read16(UART_BASE + 0x200);
}


/* SPI driver */

#define SPI_BASE	0xbf010000
#define SPI_TRXFIFO	(SPI_BASE + 0x000)
#define SPI_TRXLEN	(SPI_BASE + 0x120)
#define SPI_CONTROL	(SPI_BASE + 0x124)
#define SPI_CONTROL_TX	3
#define SPI_CONTROL_RX	5
#define SPI_CONTROL_CMDLEN_SHIFT 16
#define SPI_STATUS	(SPI_BASE + 0x140)
#define SPI_STATUS_BUSY BIT(16)
#define SPI_STATUS_RXLVL 0x0000003f
#define SPI_STATUS_TXLVL 0x00003f00
#define SPI_STATUS_TXLVL_HIGH 0x00003f00
#define SPI_CMDFIFO	(SPI_BASE + 0x148)

static void spi_init(void)
{
	write32(SPI_CONTROL, 0x200);
}

static bool spi_can_tx(void)
{
	return (read32(SPI_STATUS) & SPI_STATUS_TXLVL) < SPI_STATUS_TXLVL_HIGH;
}

static bool spi_can_rx(void)
{
	return (read32(SPI_STATUS) & SPI_STATUS_RXLVL) != 0;
}

static void put_u8(uint8_t x);
static uint8_t get_u8(void);

static void spi_transfer(size_t cmdlen, size_t txlen, size_t rxlen)
{
	uint32_t control = cmdlen << SPI_CONTROL_CMDLEN_SHIFT;
	if (txlen) {
		control |= SPI_CONTROL_TX;
		write32(SPI_TRXLEN, txlen);
	} else {
		control |= SPI_CONTROL_RX;
		write32(SPI_TRXLEN, rxlen);
	}
	write32(SPI_CONTROL, read32(SPI_CONTROL) & 0x1e00);
	write32(SPI_CONTROL, read32(SPI_CONTROL) | control);

	for (size_t i = 0; i < cmdlen; i++)
		write32(SPI_CMDFIFO, get_u8());

	while (txlen) {
		if (spi_can_tx()) {
			uint32_t word = 0;
			size_t bytes = min(4, txlen);

			for (size_t i = 0; i < bytes; i++)
				word |= get_u8() << i * 8;

			write32(SPI_TRXFIFO, word);
			txlen -= bytes;
		}
	}

	while (rxlen) {
		if (read32(SPI_STATUS) & SPI_STATUS_RXLVL) {
			uint32_t word = read32(SPI_TRXFIFO);
			size_t bytes = min(4, rxlen);

			for (size_t i = 0; i < bytes; i++)
				put_u8(word >> i * 8);

			rxlen -= bytes;
		}
	}

	while (read32(SPI_STATUS) & SPI_STATUS_BUSY)
		;
}


/* Serprog */

// Command definitions from flashrom's serprog.h
#define S_ACK             0x06
#define S_NAK             0x15
#define S_CMD_NOP         0x00   // No operation
#define S_CMD_Q_IFACE     0x01   // Query interface version
#define S_CMD_Q_CMDMAP    0x02   // Query supported commands bitmap
#define S_CMD_Q_PGMNAME   0x03   // Query programmer name
#define S_CMD_Q_SERBUF    0x04   // Query Serial Buffer Size
#define S_CMD_Q_BUSTYPE   0x05   // Query supported bustypes
#define S_CMD_Q_CHIPSIZE  0x06   // Query supported chipsize (2^n format)
#define S_CMD_Q_OPBUF     0x07   // Query operation buffer size
#define S_CMD_Q_WRNMAXLEN 0x08   // Query Write to opbuf: Write-N maximum length
#define S_CMD_R_BYTE      0x09   // Read a single byte
#define S_CMD_R_NBYTES    0x0A   // Read n bytes
#define S_CMD_O_INIT      0x0B   // Initialize operation buffer
#define S_CMD_O_WRITEB    0x0C   // Write opbuf: Write byte with address
#define S_CMD_O_WRITEN    0x0D   // Write to opbuf: Write-N
#define S_CMD_O_DELAY     0x0E   // Write opbuf: udelay
#define S_CMD_O_EXEC      0x0F   // Execute operation buffer
#define S_CMD_SYNCNOP     0x10   // Special no-operation that returns NAK+ACK
#define S_CMD_Q_RDNMAXLEN 0x11   // Query read-n maximum length
#define S_CMD_S_BUSTYPE   0x12   // Set used bustype(s).
#define S_CMD_O_SPIOP     0x13   // Perform SPI operation.
#define S_CMD_S_SPI_FREQ  0x14   // Set SPI clock frequency
#define S_CMD_S_PIN_STATE 0x15   // Enable/disable output drivers

#define CMDMAP_VALUE \
              BIT(S_CMD_NOP)       | \
              BIT(S_CMD_Q_IFACE)   | \
              BIT(S_CMD_Q_CMDMAP)  | \
              BIT(S_CMD_Q_PGMNAME) | \
              BIT(S_CMD_Q_SERBUF)  | \
              BIT(S_CMD_Q_BUSTYPE) | \
              BIT(S_CMD_SYNCNOP)   | \
              BIT(S_CMD_O_SPIOP)   | \
              BIT(S_CMD_S_BUSTYPE) | \
              BIT(S_CMD_S_PIN_STATE)

#define BUS_SPI BIT(3)

static const uint8_t progname[16] = "M88CS8001 boot1";

static void put_u8(uint8_t x) { uart_tx(x); }
static void put_u16(uint16_t x) { put_u8(x);  put_u8(x >> 8);   }
static void put_u24(uint32_t x) { put_u16(x); put_u8(x >> 16);  }
static void put_u32(uint32_t x) { put_u16(x); put_u16(x >> 16); }

static void put_array(const uint8_t *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		put_u8(data[i]);
}

static void ack(void) { put_u8(S_ACK); }
static void nak(void) { put_u8(S_NAK); }

static uint8_t get_u8(void) { return uart_rx(); }

static uint16_t get_u16(void) {
	uint16_t x;
	x  = get_u8();
	x |= get_u8() << 8;
	return x;
}

static uint32_t get_u24(void) {
	uint32_t x;
	x  = get_u16();
	x |= get_u8() << 16;
	return x;
}

void main(void) {
	spi_init();

	while (true) {
		uint8_t cmd = get_u8();

		switch (cmd) {
		case S_CMD_NOP:
			ack();
			break;
		case S_CMD_SYNCNOP:
			nak();
			ack();
			break;
		case S_CMD_Q_IFACE:
			// return interface version 1
			ack();
			put_u16(1);
			break;
		case S_CMD_Q_CMDMAP:
			ack();
			put_u32(CMDMAP_VALUE);
			for (int i = 4; i < 32; i++)
				put_u8(0);
			break;
		case S_CMD_Q_BUSTYPE:
			ack();
			put_u8(BUS_SPI);
			break;
		case S_CMD_Q_PGMNAME:
			ack();
			put_array(progname, 16);
			break;
		case S_CMD_S_BUSTYPE:
			uint8_t type = get_u8();
			if (type == BUS_SPI)
				ack();
			else
				nak();
			break;
		case S_CMD_Q_SERBUF:
			// pretend we have all the RAM
			ack();
			put_u16(0xffff);
			break;
		case S_CMD_S_PIN_STATE:
			/* uint8_t on = */ get_u8();
			ack();
			break;
		case S_CMD_O_SPIOP:
			uint32_t slen = get_u24();
			uint32_t rlen = get_u24();
			ack();
			if (rlen)
			    spi_transfer(slen, 0, rlen);
			else
			    spi_transfer(0, slen, 0);
			break;
		default:
			put_u8(S_NAK);
			break;
		}
	}
}
