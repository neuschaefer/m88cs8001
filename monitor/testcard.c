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
#define LPX(x, y) ((x) + 1920 * (y))

/* Chroma pixel coordinates:
 * - Rows are subsampled, each row in the chroma buffer is applied to two rows
 *   of pixels.
 * - Columns are subsampled: Every two pixels currespond to two bytes
 */
#define CPX1(x, y) (((x) & ~1) + (1920 * ((y) / 2)))
#define CPX2(x, y) (((x) |  1) + (1920 * ((y) / 2)))

static void main(void)
{
	uint8_t *LUMA   = (void *)0xa1fe77e0;
	uint8_t *CHROMA = (void *)0xa21e5840;

	memset(LUMA, 0, 1920 * 1080);
	memset(CHROMA, 0x80, 1920 * 1080);

	/* Border */
	for (int i = 0; i < 1026; i++) {
		LUMA[LPX(27 + i,    27)]        = 0xff;  // top
		LUMA[LPX(27 + i,    28 + 1024)] = 0xff;  // bottom
		LUMA[LPX(27,        27 + i)]    = 0xff;  // left
		LUMA[LPX(28 + 1024, 27 + i)]    = 0xff;  // right
	}

	/* First square: luma vs. chroma */
	for (int x = 0; x < 1024; x++)
	for (int y = 0; y < 1024; y++) {
		LUMA  [LPX (x + 28, y + 28)] = x / 4;
		CHROMA[CPX1(x + 28, y + 28)] = y / 128;
		CHROMA[CPX2(x + 28, y + 28)] = (y % 128) * 2;
	}

	/* Chroma space at luma=0.5 */
	for (int i = 0; i < 514; i++) {
		LUMA[LPX(1100 + i,   27)]        = 0xff;  // top
		LUMA[LPX(1100 + i,   28 + 512)]  = 0xff;  // bottom
		LUMA[LPX(1099,       27 + i)]    = 0xff;  // left
		LUMA[LPX(1100 + 512, 27 + i)]    = 0xff;  // right
	}
	for (int x = 0; x < 512; x++)
	for (int y = 0; y < 512; y++) {
		LUMA  [LPX (x + 1100, y + 28)] = 0x80;
		CHROMA[CPX1(x + 1100, y + 28)] = x / 2;
		CHROMA[CPX2(x + 1100, y + 28)] = y / 2;
	}
}
