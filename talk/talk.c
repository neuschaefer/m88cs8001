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


/* Timer driver */

/* This register increments at roughly 3.275 MHz */
#define TIMER_REG	0xbf44308c
#define TIMER_FREQ	3275000

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

#define I2C_BASE	0xbf158000  /* I2C 2, but we only need that one */
#define I2C_STATUS	(I2C_BASE + 0x04)
#define I2C_WRITE	(I2C_BASE + 0x08)
#define I2C_READ	(I2C_BASE + 0x0c)
#define I2C_CONTROL	(I2C_BASE + 0x14)
#define I2C_CONTROL_DONE	0x04
#define I2C_CONTROL_SPECIAL	0x04
#define I2C_CONTROL_READ	0x10
#define I2C_CONTROL_WRITE	0x20
#define I2C_CONTROL_STOP	0x40
#define I2C_CONTROL_START	0x80

static void i2c_finish(void)
{
	while (read8(I2C_CONTROL) != I2C_CONTROL_DONE)
		;

	uint8_t status = read8(I2C_STATUS);
	if (status & 0xa0) {
		putstr("I2C error: ");
		put_hex8(status);
		putchar('\n');
	}
}

static void i2c_write(uint8_t value, bool start)
{
	uint8_t control = I2C_CONTROL_WRITE;

	if (start)
		control |= I2C_CONTROL_START;

	write8(I2C_WRITE, value);
	write8(I2C_CONTROL, control);
	i2c_finish();
}

static void i2c_stop(void)
{
	write8(I2C_CONTROL, I2C_CONTROL_STOP);
	i2c_finish();
}

static uint8_t i2c_read(bool special)
{
	uint8_t control = I2C_CONTROL_READ;
	if (special)
		control |= I2C_CONTROL_SPECIAL;

	write8(I2C_CONTROL, control);
	i2c_finish();

	return read8(I2C_READ);
}

static void fp_write(uint8_t addr, uint8_t value)
{
	i2c_write((addr | 0x20) << 1, true);
	i2c_write(value, false);
	i2c_stop();
}

static void fp_enable(void)
{
	fp_write(4, 0x41);
}

static void fp_set_digit(int num, uint8_t value)
{
	fp_write(0x14 + num, value);
}

static uint8_t fp_get_key(void)
{
	uint8_t value;

	i2c_write((7 | 0x20) << 1 | 1, true);
	value = i2c_read(true);
	i2c_stop();

	return value;
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
	COLOR_GREY_F0 = YCbCr(0xf0, 0x80, 0x80),
	COLOR_GREY_D0 = YCbCr(0xd0, 0x80, 0x80),
	COLOR_RED = YCbCr(0x20, 0x80, 0xff),
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
	chroma[(x + CHROMA_WIDTH * y) * 2]     = color_get_cr(color);
	chroma[(x + CHROMA_WIDTH * y) * 2 + 1] = color_get_cb(color);
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

	arena_make_permanent();
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

extern const struct font font_cozette;
const struct font *font_default = &font_cozette;

static const struct chardef *font_find_char(const struct font *font, int c)
{
	if (c >= font->first_char &&
	    c - font->first_char < font->num_chars)
		return &font->chars[c - font->first_char];
	return NULL;
}

static struct bbx chardef_get_bbx(const struct font *font, const struct chardef *cdef)
{
	struct bbx bbx = {
		.w = cdef->bbx.w + font->bbx_bias.w,
		.h = cdef->bbx.h + font->bbx_bias.h,
		.xoff = cdef->bbx.xoff + font->bbx_bias.xoff,
		.yoff = cdef->bbx.yoff + font->bbx_bias.yoff,
	};
	return bbx;
}

static void font_draw_char(const struct font *font, FB fb, int x_start, int y_start, int scale, color_t fg, color_t bg, int c)
{
	const struct chardef *cdef = font_find_char(font, c);

	if (!cdef)
		cdef = font_find_char(font, '?');

	if (!cdef)
		return;

	struct bbx bbx = chardef_get_bbx(font, cdef);

	for (int i = 0; i < cdef->bm_len; i++) {
		uint8_t bm = font->bitmaps[cdef->bm_off + i];

		for (int b = 0; b < 8; b++) {
			int x = x_start + (b + bbx.xoff) * scale;
			int y = y_start + (i - bbx.yoff - bbx.h) * scale;

			if (bm & BIT(7 - b))
				fb_draw_px_scaled(fb, x, y, scale, fg);
		}
	}

	// TODO: draw background
	(void)bg;
}

static void font_draw(const struct font *font, FB fb, int x_start, int y_start, int scale, color_t fg, color_t bg, const char *string)
{
	int x = x_start, y = y_start;

	for (const char *p = string; *p; p++) {
		if (*p == '\n') {
			x = x_start;
			y += font->font_bbx.h * scale;
		} else {
			font_draw_char(font, fb, x, y, scale, fg, bg, *p);
			x += font->norm_space * scale;
		}
	}
}

static void font_measure(const struct font *font, int *width, int *height, int scale, const char *string)
{
	int x = 0, y = 0;
	*height = 0;
	*width = 0;

	for (const char *p = string; *p; p++) {
		if (*p == '\n') {
			x = 0;
			y += font->font_bbx.h * scale;
		} else {
			x += font->norm_space * scale;
			*height = max(y + font->font_bbx.h * scale, *height);
			*width = max(x, *width);
		}
	}
}


#define TITLE_X 30
#define TITLE_Y 100
#define TITLE_SCALE 10
#define TITLE_Y_SHADOW_OFFSET 5
#define TITLE_X_SHADOW_OFFSET 5

static void font_draw_title(const struct font *font, FB fb, color_t fg, color_t shadow, char *string)
{
	font_draw(font, fb, TITLE_X + TITLE_X_SHADOW_OFFSET, TITLE_Y + TITLE_Y_SHADOW_OFFSET,
			TITLE_SCALE, shadow, TRANSPARENT, string);
	font_draw(font, fb, TITLE_X, TITLE_Y, TITLE_SCALE, fg, TRANSPARENT, string);
}

#define HEADLINE_X 20
#define HEADLINE_Y 80
#define HEADLINE_SCALE 6
#define HEADLINE_Y_SHADOW_OFFSET 3
#define HEADLINE_X_SHADOW_OFFSET 3

static void font_draw_headline(const struct font *font, FB fb, color_t fg, color_t shadow, char *string)
{
	font_draw(font, fb, HEADLINE_X + HEADLINE_X_SHADOW_OFFSET, HEADLINE_Y + HEADLINE_Y_SHADOW_OFFSET,
			HEADLINE_SCALE, shadow, TRANSPARENT, string);
	font_draw(font, fb, HEADLINE_X, HEADLINE_Y, HEADLINE_SCALE, fg, TRANSPARENT, string);
}

#define MAIN_TEXT_X 40
#define MAIN_TEXT_Y 140


static void font_draw_window(const struct font *font, FB fb, int x, int y, int width, int height,
			     color_t frame_fg, color_t window_bg, color_t title_fg, color_t title_bg,
			     const char *title)
{
	x -= 2; width += 4;
	y -= font->font_bbx.h;
	height += 4;

	// Background
	for (int j = - font->font_bbx.h; j <= height; j++)
	for (int i = 0; i < width; i++) {
		if (j < 0)
			fb_draw_px(fb, x + i, y + j, title_bg);
		else
			fb_draw_px(fb, x + i, y + j, window_bg);
	}

	// Horizontal frame
	for (int i = 0; i < width; i++) {
		fb_draw_px(fb, x + i, y - font->font_bbx.h, frame_fg);
		fb_draw_px(fb, x + i, y,                    frame_fg);
		fb_draw_px(fb, x + i, y + height,           frame_fg);
	}

	// Vertical frame
	for (int i = - font->font_bbx.h; i <= height; i++) {
		fb_draw_px(fb, x,         y + i, frame_fg);
		fb_draw_px(fb, x + width, y + i, frame_fg);
	}

	// Title
	font_draw(font, fb, x + 2, y - 2, 1, title_fg, TRANSPARENT, title);

	// X
	for (int i = 2; i <= font->font_bbx.h - 2; i++) {
		fb_draw_px(fb, x + width - 1 - i, y - font->font_bbx.h + i, frame_fg);
		fb_draw_px(fb, x + width - 1 - i, y - i,                    frame_fg);
	}
}

static void font_draw_text_window(const struct font *font, FB fb, int x, int y, const char *title, const char *text)
{
	int width = 0, height = 0;
	font_measure(font, &width, &height, 1, text);

	int twidth = 0, theight = 0;
	font_measure(font, &twidth, &theight, 1, title);

	width = max(width, twidth + 15);
	(void)theight;

	font_draw_window(font, fb, x, y, width, height,
			 COLOR_BLACK, COLOR_GREY_F0, COLOR_BLACK, COLOR_GREY_D0, title);

	font_draw(font, fb, x, y, 1, COLOR_BLACK, TRANSPARENT, text);
}

/* Other drawing routines */
//   - rahmen etc., für einfache grafik reichts ja
//   - draw_window
//   - draw_hexdump
//   - draw_ghidra


struct slide {
	void (*init)(void *);
	void (*update)(void *);
	void (*exit)(void *);
	size_t ctx_size;
};


#include "slide_title.c"
#include "slide_parttable.c"
#include "slide_endcard.c"

static const struct slide *const slides[] = {
	&slide_title,
	&slide_part0,
	&slide_part1,
	&slide_part2,
	&slide_part3,
	&slide_endcard,
};

static int current_slide, next_slide;
static bool quit_requested;
static void *slide_ctx;
static uint8_t previous_fp_key = 0;

static void change_slide(int target)
{
	if (target >= 0 &&
	    target < (int)ARRAY_LENGTH(slides))
		next_slide = target;
}

static void check_inputs(void)
{
	while (uart_rx_level() != 0) {
		int c = uart_rx();

		switch (c) {
		case 'q':
			quit_requested = true;
			break;
		case 'j':
			change_slide(current_slide + 1);
			break;
		case 'k':
			change_slide(current_slide - 1);
			break;
		}
	}

	uint8_t fp_key = fp_get_key();
	if (fp_key != previous_fp_key) {
		//putstr("FP key: "); put_hex8(fp_key); putchar('\n');
		if (fp_key & 0x40) {
			switch (fp_key & 0x3f) {
			case 0x1f: /* left */
				change_slide(current_slide - 1);
				break;
			case 0x07: /* right */
				change_slide(current_slide + 1);
				break;
			case 0x17: /* power */
				break;
			}
		}

		previous_fp_key = fp_key;
	}
}

uint32_t last_frame;

static bool waiting_for_next_frame(void)
{
	uint32_t now = timer_get();

	if ((now - last_frame) < TIMER_FREQ / 30)
		return true;

	last_frame = now;
	return false;
}

static void main_loop(void)
{
	current_slide = -1;

	while (!quit_requested) {
		const struct slide *slide = (current_slide >= 0)? slides[current_slide] : NULL;

		if (next_slide != current_slide) {
			// Shut down the old slide
			if (slide && slide->exit)
				slide->exit(slide_ctx);
			arena_clear();

			// Initialize the next slide
			putstr("Slide "); put_hex8(next_slide); putchar('\n');
			current_slide = next_slide;
			slide = slides[current_slide];
			slide_ctx = slide->ctx_size? arena_zalloc(slide->ctx_size) : NULL;
			if (slide->init)
				slide->init(slide_ctx);
		}

		if (slide->update)
			slide->update(slide_ctx);

		while (waiting_for_next_frame())
			check_inputs();
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

	fp_set_digit(0, 0b1110110);
	fp_set_digit(1, 0b1110111);
	fp_set_digit(2, 0b0111001);
	fp_set_digit(3, 0b0111001);

	main_loop();
}
