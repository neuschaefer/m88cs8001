// SPDX-License-Identifier: MIT
//
// Presentation on the sat receiver

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "font.h"

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(x) (1UL << (x))
#define min(a,b) (((a) < (b))? (a) : (b))
#define max(a,b) (((a) > (b))? (a) : (b))
#define KiB (1 << 10)
#define MiB (1 << 20)
#define GiB (1 << 30)
#define MEM_C(x)   ((void *)((((uint32_t)(x)) & 0x1fffffff) | 0x80000000))
#define MEM_UC(x)  ((void *)((((uint32_t)(x)) & 0x1fffffff) | 0xa0000000))
#define MEM_DMA(x) (((uint32_t)(x)) & 0x1fffffff)

/* MMIO accessors */

static uint8_t  read8(unsigned long addr)  { return *(volatile uint8_t *)addr; }
static uint16_t read16(unsigned long addr) { return *(volatile uint16_t *)addr; }
static uint32_t read32(unsigned long addr) { return *(volatile uint32_t *)addr; }

static void write8(unsigned long addr, uint8_t value)   { *(volatile uint8_t *)addr = value; }
static void write16(unsigned long addr, uint16_t value) { *(volatile uint16_t *)addr = value; }
static void write32(unsigned long addr, uint32_t value) { *(volatile uint32_t *)addr = value; }


/* Cache manipulation */

#define CACHE_LINE 32
#define CACHE_LINE_MASK (CACHE_LINE - 1)

extern char synci_line[1];
static void (* synci_line_p)(unsigned long p) = (void *)synci_line;
static void cache_flush_range(unsigned long addr, size_t len)
{
	for (unsigned long p = addr & ~CACHE_LINE_MASK; p < addr + len; p += CACHE_LINE)
		synci_line_p(p);
}


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


/* Timer driver */

/* This register increments at roughly 3.275 MHz */
#define TIMER_REG	0xbf44308c

static uint32_t timer_get()
{
	return read32(TIMER_REG);
}

static uint32_t check_timeout(uint32_t start, uint32_t period_ms)
{
	return (timer_get() - start) >= 3275 * period_ms;
}

static void sleep_ticks(uint32_t ticks)
{
	uint32_t start = timer_get();

	while ((timer_get() - start) < ticks)
		for (int i = 0; i < 100000; i++)
			asm volatile ("nop");
}


/* I2C/frontpanel driver */

// TODO: port from interact.py
// - vor/zurück mit den Tasten


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

/* Get a character from the UART */
static int getchar(void)
{
	return uart_rx();
}


/* String functions */

static size_t strlen(const char *s)
{
	size_t len = 0;

	for (const char *p = s; *p; p++)
		len++;

	return len;
}

static int strncmp(const char *a, const char *b, size_t n)
{
	for (size_t i = 0; i < n && a[i] && b[i]; i++) {
		if (a[i] != b[i])
			return (int)a[i] - (int)b[i];
	}

	return 0;
}

static void *memcpy(void *d, const void *s, size_t n)
{
	char *dc = d;
	const char *sc = s;

	for (size_t i = 0; i < n; i++)
		dc[i] = sc[i];

	return d;
}

static void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;

	for (size_t i = 0; i < n; i++)
		*p++ = c;

	return s;
}


/* Arena allocator */

static uint8_t *arena_start;
static uint8_t *arena_next;
static uint8_t *arena_limit;

static void arena_init(void) {
	arena_start = MEM_C(1*MiB);
	arena_next = arena_start;
	arena_limit = MEM_C(60*MiB);
// - memory management:
//   - leave area around original framebuffers untouched
//   - arena allocator
//   - luma/chroma buffers A/B
//     - luma_get_free()
//     - luma_present(luma)
//     - typedef struct framebuffer { void *luma; void *chroma; } FB;
//     - FB fb_get_free();
//     - fb_present(FB fb);
	//...
}

static void *arena_alloc(size_t size) {
	void *p = arena_next;

	// Advance the arena pointer past the new object, but observe 8-byte alignment
	arena_next += (size + 8) & ~7;

	return p;
}

static void *arena_zalloc(size_t size) {
	void *p = arena_alloc(size);
	memset(p, 0, size);
	return p;
}

static void arena_clear(void) {
	arena_next = arena_start;
}

// After calling this function, arena_clear() will not remove previous allocations.
static void arena_make_permanent(void) {
	arena_start = arena_next;
}


/* Colors */

typedef int color_t;
#define TRANSPARENT	(-1)
#define YCbCr(y,cb,cr)	((y) << 16 | (cb) << 8 | (cr) << 0)
static int color_is_transparent(color_t c)  { return c == TRANSPARENT; }
static int color_get_y(color_t c)           { return (c >> 16) & 0xff; }
static int color_get_cb(color_t c)          { return (c >>  8) & 0xff; }
static int color_get_cr(color_t c)          { return (c >>  0) & 0xff; }

enum {
	COLOR_WHITE = YCbCr(0xff, 0x80, 0x80),
	COLOR_BLACK = YCbCr(0x00, 0x80, 0x80),
	COLOR_GREY = YCbCr(0x80, 0x80, 0x80),
};


/* Frame buffer management */

#define DISP_BASE	0xbf441000
#define DISP_LUMA	(DISP_BASE + 0x20)
#define DISP_CHROMA	(DISP_BASE + 0x1c)
#define LUMA_WIDTH	1920
#define LUMA_HEIGHT	1080
#define LUMA_SIZE	(LUMA_WIDTH * LUMA_HEIGHT)
#define CHROMA_WIDTH	960
#define CHROMA_HEIGHT	540
#define CHROMA_SIZE	(CHROMA_WIDTH * CHROMA_HEIGHT * 2)
#define FB_WIDTH	CHROMA_WIDTH
#define FB_HEIGHT	CHROMA_HEIGHT

static void *luma_get_active(void)   { return MEM_C(read32(DISP_LUMA)   << 3); }
static void *chroma_get_active(void) { return MEM_C(read32(DISP_CHROMA) << 3); }

static void luma_set_active(void *luma)     { write32(DISP_LUMA,   MEM_DMA(luma)   >> 3); }
static void chroma_set_active(void *chroma) { write32(DISP_CHROMA, MEM_DMA(chroma) >> 3); }

static void luma_set(uint8_t *luma, int x, int y, color_t color)
{
	luma[x + LUMA_WIDTH * y] = color_get_y(color);
}

static void chroma_set(uint8_t *chroma, int x, int y, color_t color)
{
	chroma[x + CHROMA_WIDTH * y * 2]     = color_get_cr(color);
	chroma[x + CHROMA_WIDTH * y * 2 + 1] = color_get_cb(color);
}

static void luma_fill(uint8_t *luma, color_t color)
{
	memset(luma, color_get_y(color), LUMA_SIZE);
}

static void chroma_fill(uint8_t *chroma, color_t color)
{
	uint8_t cr = color_get_cr(color);
	uint8_t cb = color_get_cb(color);

	for (size_t i = 0; i < CHROMA_SIZE; i += 2) {
		chroma[i]   = cr;
		chroma[i+1] = cb;
	}
}


typedef struct framebuffer {
	void *luma;
	void *chroma;
} FB;

static FB fb_get_active(void) {
	FB fb = {
		.luma = luma_get_active(),
		.chroma = chroma_get_active(),
	};
	return fb;
}

static void fb_present(FB fb) {
	cache_flush_range((uint32_t) fb.luma, LUMA_SIZE);
	cache_flush_range((uint32_t) fb.chroma, CHROMA_SIZE);
	luma_set_active(fb.luma);
	chroma_set_active(fb.chroma);
}

FB fb_bootsplash;
void *luma_buffers[2];
void *chroma_buffers[2];

static void fb_init(void) {
	// TODO: Identify bootsplash, make backups, etc.
	fb_bootsplash = fb_get_active();
	// set up pool of frame buffers

	for (size_t i = 0; i < ARRAY_LENGTH(luma_buffers); i++) {
		luma_buffers[i] = arena_alloc(LUMA_SIZE);
		chroma_buffers[i] = arena_alloc(CHROMA_SIZE);
	};
}

static void *luma_get_free(void)
{
	if (luma_get_active() != luma_buffers[0])
		return luma_buffers[0];
	else
		return luma_buffers[1];
}

static void *chroma_get_free(void)
{
	if (chroma_get_active() != chroma_buffers[0])
		return chroma_buffers[0];
	else
		return chroma_buffers[1];
}

static FB fb_get_free(void) {
	FB fb = {
		.luma = luma_get_free(),
		.chroma = chroma_get_free(),
	};
	return fb;
}

static void fb_copy(FB fb, FB source)
{
	memcpy(fb.luma, source.luma, LUMA_SIZE);
	memcpy(fb.chroma, source.chroma, CHROMA_SIZE);
}

static void fb_fill(FB fb, color_t color)
{
	luma_fill(fb.luma, color);
	chroma_fill(fb.chroma, color);
}

static void fb_draw_px(FB fb, int x, int y, color_t color)
{
	if (color_is_transparent(color) || x >= FB_WIDTH || y >= FB_HEIGHT)
		return;

	luma_set(fb.luma, x * 2,     y * 2,     color);
	luma_set(fb.luma, x * 2 + 1, y * 2,     color);
	luma_set(fb.luma, x * 2 + 1, y * 2 + 1, color);
	luma_set(fb.luma, x * 2,     y * 2 + 1, color);
	chroma_set(fb.chroma, x, y, color);
}

static void fb_draw_px_scaled(FB fb, int x, int y, int scale, color_t color)
{
	for (int i = 0; i < scale; i++)
		for (int j = 0; j < scale; j++)
			fb_draw_px(fb, x + j, y + i, color);
}


/* Font rendering */

extern struct font font_cozette;
#define font_default (&font_cozette)

static void font_draw_char(struct font *font, FB fb, int x, int y, int scale, color_t fg, color_t bg, int c)
{
	// TODO: draw background
	// TODO: draw bitmap

	(void)font;
	(void)fb;
	(void)x;
	(void)y;
	(void)scale;
	(void)fg;
	(void)bg;
	(void)c;
}

static void font_draw(struct font *font, FB fb, int x, int y, int scale, color_t fg, color_t bg, char *string)
{
	// TODO:
	// for each char:
	//  - if normal:
	//    draw, x += width
	//  - if '\n':
	//    reset x, y += height

	(void)font;
	(void)fb;
	(void)x;
	(void)y;
	(void)scale;
	(void)fg;
	(void)bg;
	(void)string;
}

#define TITLE_X 30
#define TITLE_Y 100
#define TITLE_SCALE 10
#define TITLE_Y_SHADOW_OFFSET 15
#define TITLE_X_SHADOW_OFFSET 15

static void font_draw_title(struct font *font, FB fb, color_t fg, color_t shadow, char *string)
{
	font_draw(font, fb, TITLE_X + TITLE_X_SHADOW_OFFSET, TITLE_Y + TITLE_Y_SHADOW_OFFSET,
			TITLE_SCALE, shadow, TRANSPARENT, string);
	font_draw(font, fb, TITLE_X, TITLE_Y, TITLE_SCALE, fg, TRANSPARENT, string);
}


/* Other drawing routines */
//   - rahmen etc., für einfache grafik reichts ja
//   - draw_window
//   - draw_hexdump
//   - draw_ghidra


// - per UART: vor, zurück, zu bestimmter seite, neuladen (X-Modem?), quit
// - Inhalt
//   - Was ist das für ein Gerät (anderes Exemplar rumgehen lassen)
//   - Zusammenfassung des ersten Blogposts
//     - partition table
//     - partition table listing
//     - code exec  (scream.S listing)
//   - lolmon
//   - wie ich nach dem Hai im RAM gesucht habe
//     - 0x1000 byte scandump
//   - 1920-byte-offset gefunden
//     - Hexdump anzeigen (einfach berechnen)
//   - Chromabuffer gefunden und verschoben
//   - Lumabuffer gefunden
//   - Testbild
//   - Framebuffer-Adressregister gefunden (Ghidra-Ausschnitt)
//     - Mock-Ghidra nachbauen
//   - Buttons
//   - Conclusion
//     - Es gibt Welten zu entdecken!
//     - 28€ hat diese Welt gekostet
//   - irgendwelche lustigen CGI-Tricks
//   - endcard
//     - danke fürs zuhören
//     - font copyright
//

struct slide {
	void (*init)(void *);
	void (*draw)(void *);
	void (*exit)(void *);
	size_t ctx_size;
};


#include "slide_title.c"
#include "slide_endcard.c"

static const struct slide *const slides[] = {
	&slide_title,
	&slide_endcard,
};

static int current_slide = -1, next_slide;
static bool quit_requested;
static void *slide_ctx;

static void check_inputs(void)
{
	while (uart_rx_level() != 0) {
		int c = uart_rx();

		if (c == 'q')
			quit_requested = true;
	}

	// TODO: button inputs
}

static bool waiting_for_next_frame(void)
{
	return false;
	// TODO
}

static void main_loop(void)
{
	while (!quit_requested) {
		const struct slide *slide = (current_slide >= 0)? slides[current_slide] : NULL;

		if (next_slide != current_slide) {
			// Shut down the old slide
			if (slide && slide->exit)
				slide->exit(slide_ctx);
			arena_clear();

			// Initialize the next slide
			current_slide = next_slide;
			slide = slides[current_slide];
			slide_ctx = slide->ctx_size? arena_zalloc(slide->ctx_size) : NULL;
			if (slide->init)
				slide->init(slide_ctx);
		}

		if (slide->draw)
			slide->draw(slide_ctx);

		check_inputs();

		while (waiting_for_next_frame())
			;
	}

	fb_present(fb_bootsplash);
}

extern char _bss_start[];
extern char _bss_end[];
char *bss_start_p = _bss_start;
char *bss_end_p = _bss_end;
void bss_init(void)
{
	memset(bss_start_p, 0, bss_end_p - bss_start_p);
}

int main(void)
{
	puts("Launching presentation...");
	bss_init();
	arena_init();
	fb_init();

	main_loop();
}
