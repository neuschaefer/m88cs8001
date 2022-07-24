// SPDX-License-Identifier: MIT
// HDMI output test card

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

static void main(void);
void start(void)
{
	main();
}

static void memset(void *s, int c, size_t n)
{
	char *p = s;

	for (size_t i = 0; i < n; i++)
		p[i] = c;
}

/* Luma pixel coordinates: One row/column per pixel */
#define YPX(x, y) ((x) + 1920 * (y))

/* Chroma pixel coordinates:
 * - Rows are subsampled, each row in the chroma buffer is applied to two rows
 *   of pixels.
 * - Columns are subsampled: Every two pixels currespond to two bytes
 */
#define CrPX(x, y) (((x) & ~1) + (1920 * ((y) / 2)))
#define CbPX(x, y) (((x) |  1) + (1920 * ((y) / 2)))

static void main(void)
{
	uint8_t *LUMA   = (void *)0xa1fe77e0;
	uint8_t *CHROMA = (void *)0xa21e5840;

	memset(LUMA, 0, 1920 * 1080);
	memset(CHROMA, 0x80, 1920 * 1080);

	/* Border */
	for (int i = 0; i < 1026; i++) {
		LUMA[YPX(27 + i,    27)]        = 0xff;  // top
		LUMA[YPX(27 + i,    28 + 1024)] = 0xff;  // bottom
		LUMA[YPX(27,        27 + i)]    = 0xff;  // left
		LUMA[YPX(28 + 1024, 27 + i)]    = 0xff;  // right
	}

	/* First square: luma vs. chroma */
	for (int x = 0; x < 1024; x++)
	for (int y = 0; y < 1024; y++) {
		int stripe = y / 114;

		LUMA  [YPX (x + 28, y + 28)] = x / 4;
		CHROMA[CrPX(x + 28, y + 28)] = (stripe % 3) * 127;
		CHROMA[CbPX(x + 28, y + 28)] = (stripe / 3) * 127;
	}

	/* Chroma space at different lumas */
	for (int panel = 0; panel < 12; panel++) {
		int X = 1100 + (panel % 3) * 260;
		int Y = 28 + (panel / 3) * 260;
		uint8_t luma = panel * 23;

		for (int i = 0; i < 256 + 2; i++) {
			LUMA[YPX(X - 1 + i, Y - 1)]     = 0xff;  // top
			LUMA[YPX(X - 1 + i, Y + 256)]   = 0xff;  // bottom
			LUMA[YPX(X - 1,     Y - 1 + i)] = 0xff;  // left
			LUMA[YPX(X + 256,   Y - 1 + i)] = 0xff;  // right
		}
		for (int x = 0; x < 256; x++)
		for (int y = 0; y < 256; y++) {
			LUMA  [YPX (x + X, y + Y)] = luma;
			CHROMA[CrPX(x + X, y + Y)] = x;
			CHROMA[CbPX(x + X, y + Y)] = y;
		}
	}
}
