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

static void spi_transfer(const uint8_t *cmdbuf, size_t cmdlen,
			 const uint8_t *txbuf, size_t txlen,
			 uint8_t *rxbuf, size_t rxlen)
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
		write32(SPI_CMDFIFO, cmdbuf[i]);

	while (txlen) {
		if (spi_can_tx()) {
			uint32_t word = 0;
			size_t bytes = min(4, txlen);

			for (size_t i = 0; i < bytes; i++)
				word |= *txbuf++ << (3-i) * 8;

			write32(SPI_TRXFIFO, word);
			txlen -= bytes;
		}
	}

	while (rxlen) {
		if (read32(SPI_STATUS) & SPI_STATUS_RXLVL) {
			uint32_t word = read32(SPI_TRXFIFO);
			size_t bytes = min(4, rxlen);

			for (size_t i = 0; i < bytes; i++)
				*rxbuf++ = word >> (3-i) * 8;

			rxlen -= bytes;
		}
	}

	while (read32(SPI_STATUS) & SPI_STATUS_BUSY)
		;
}

static void flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
	uint8_t cmd[4] = {
		0x03,
		addr >> 16,
		addr >> 8,
		addr
	};
	spi_transfer(cmd, sizeof(cmd), NULL, 0, buf, size);
}


/* Watchdog timer driver */

static void wdt_arm(uint32_t cycles) {
	write32(0xbf100100, cycles);
	write32(0xbf100104, 0x12345678);
	write32(0xbf100108, 1);
}

static bool reset_cause_was_wdt(void) {
	return read16(0xbf100114) > 0;
}


/* Linux slide */

struct linux_ctx {
	uint32_t flash_start, flash_end, flash_current;
	uint8_t *mem_start, *mem;
	int old_x;
};

static void linux_init(void *ctx) {
	struct linux_ctx *l = ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "But does it run Linux?");
	font_draw_main(font_default, fb, 0, "- Let's see...");

	l->flash_start = l->flash_current = 0x110000;
	l->flash_end = 0x400000;
	l->mem_start = l->mem = MEM_C(0x81000000);
	l->old_x = 30;

	fb_present(fb);
}

static void linux_update(void *ctx) {
	struct linux_ctx *l = ctx;
	size_t chunk = 16 * KiB;

	FB fb = fb_get_active();

	if (l->flash_current < l->flash_end) {
		putchar('.');

		flash_read(l->flash_current, l->mem, chunk);
		l->flash_current += chunk;
		l->mem += chunk;

		// draw progress bar
		int x = 30 + 900 * (l->flash_current - l->flash_start) / (l->flash_end - l->flash_start);
		fb_fill_rect(fb, l->old_x, MAIN_TEXT_Y + 90, x - l->old_x, 40, COLOR_GREEN);
		l->old_x = x;
	} else {
		font_draw_main(font_default, fb, 3, "                   R E A D Y !");
	}

	fb_present(fb);
}

static void linux_action(void *ctx) {
	struct linux_ctx *l = ctx;

	if (l->flash_current < l->flash_end) {
		puts("not ready");
		return;
	}
	cache_flush_range(0x80000000, 64 * MiB);

	puts("lets go");
	chroma_set_active(fb_bootsplash.chroma);
	memset(MEM_C(30 * MiB), 0, 2 * MiB);
	luma_set_active(MEM_C(30 * MiB));

	wdt_arm(0xffffffff); // about 7 seconds until it reboots the system

	do_call_p((uint32_t)l->mem_start, 0, ~0u, 0);
}

static const struct slide slide_linux0 = {
	.init = linux_init,
	.update = linux_update,
	.action = linux_action,
	.ctx_size = sizeof(struct linux_ctx),
};

static void linux_init_yes(void *ctx) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "But does it run Linux?");
	font_draw_main(font_default, fb, 0, "- Let's see...");
	font_draw_main(font_default, fb, 1, "- Yes!");

	fb_present(fb);
}

static const struct slide slide_linux1 = { .init = linux_init_yes };
