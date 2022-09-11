// SPDX-License-Identifier: MIT

#define ENDCARD_SCALE 15
#define ENDCARD_LUMA_HEIGHT (LUMA_HEIGHT / ENDCARD_SCALE)
#define ENDCARD_LUMA_WIDTH (LUMA_WIDTH / ENDCARD_SCALE)
#define ENDCARD_CHROMA_HEIGHT (CHROMA_HEIGHT / ENDCARD_SCALE)
#define ENDCARD_CHROMA_WIDTH (CHROMA_WIDTH / ENDCARD_SCALE)
#define ENDCARD_HEIGHT (FB_HEIGHT / ENDCARD_SCALE)
#define ENDCARD_WIDTH (FB_WIDTH / ENDCARD_SCALE)

struct endcard {
	// Scaled version of the boot splash, for use between names
	uint8_t logo_luma[ENDCARD_LUMA_HEIGHT * ENDCARD_LUMA_WIDTH];
	uint8_t logo_chroma[ENDCARD_CHROMA_WIDTH * ENDCARD_CHROMA_HEIGHT * 2];
	size_t name_index;
	int px;
};

static void scale_down(struct endcard *ec, FB fb)
{
	for (int Y = 0; Y < ENDCARD_LUMA_HEIGHT; Y++)
	for (int X = 0; X < ENDCARD_LUMA_WIDTH; X++) {
		uint32_t sum = 0;
		for (int yo = 0; yo < ENDCARD_SCALE; yo++)
		for (int xo = 0; xo < ENDCARD_SCALE; xo++) {
			int x = X * ENDCARD_SCALE + xo;
			int y = Y * ENDCARD_SCALE + yo;
			sum += luma_get(fb.luma, x, y);
		}
		ec->logo_luma[X + ENDCARD_LUMA_WIDTH * Y] = sum / (ENDCARD_SCALE * ENDCARD_SCALE);
	}

	for (int Y = 0; Y < ENDCARD_CHROMA_HEIGHT; Y++)
	for (int X = 0; X < ENDCARD_CHROMA_WIDTH; X++) {
		uint32_t sum0 = 0;
		uint32_t sum1 = 0;
		for (int yo = 0; yo < ENDCARD_SCALE; yo++)
		for (int xo = 0; xo < ENDCARD_SCALE; xo++) {
			int x = X * ENDCARD_SCALE + xo;
			int y = Y * ENDCARD_SCALE + yo;
			color_t color = chroma_get(fb.chroma, x, y);
			sum0 += color_get_cr(color);
			sum1 += color_get_cb(color);
		}
		ec->logo_chroma[(X + ENDCARD_CHROMA_WIDTH * Y) * 2    ] = sum0 / (ENDCARD_SCALE * ENDCARD_SCALE);
		ec->logo_chroma[(X + ENDCARD_CHROMA_WIDTH * Y) * 2 + 1] = sum1 / (ENDCARD_SCALE * ENDCARD_SCALE);
	}
}

static void endcard_logo_present(struct endcard *ec, FB fb, int x, int y)
{
	uint8_t *luma   = fb.luma;
	uint8_t *chroma = fb.chroma;

	if (x > FB_WIDTH || y > FB_HEIGHT)
		return;

	for (int Y = 0; Y < ENDCARD_LUMA_HEIGHT && Y+2*y < LUMA_HEIGHT; Y++)
	for (int X = 0; X < ENDCARD_LUMA_WIDTH  && X+2*x < LUMA_WIDTH; X++) {
		if (X+2*x >= 0)
			luma[X+2*x + LUMA_WIDTH * (Y+2*y)] = ec->logo_luma[X + ENDCARD_LUMA_WIDTH * Y];
	}

	for (int Y = 0; Y < ENDCARD_CHROMA_HEIGHT && Y+y < CHROMA_HEIGHT; Y++)
	for (int X = 0; X < ENDCARD_CHROMA_WIDTH  && X+x < CHROMA_WIDTH; X++) {
		if (X+x >= 0) {
			chroma[(X+x + CHROMA_WIDTH * (Y+y)) * 2]     = ec->logo_chroma[(X + ENDCARD_CHROMA_WIDTH * Y) * 2    ];
			chroma[(X+x + CHROMA_WIDTH * (Y+y)) * 2 + 1] = ec->logo_chroma[(X + ENDCARD_CHROMA_WIDTH * Y) * 2 + 1];
		}
	}
}

// TODO: move somewhere else
static void draw_cccac(FB fb, int x, int y, int tile) {
	fb_fill_rect(fb, x, y, 5*tile, 4*tile, COLOR_GREY);
	for (int i = 0; i < 20; i++) {
		int col = i / 4;
		int row = i % 4;

		int bit = 0xCCCAC & BIT(20-1 - row - 4 * col);
		fb_fill_rect(fb, x + col*tile + 2, y + row*tile + 2, tile-4, tile-4,
				bit? COLOR_WHITE:COLOR_BLACK);
	}
}

static const char draw_cccac_source[] =
"static void draw_cccac(FB fb, int x, int y, int tile) {\n"
"        fb_fill_rect(fb, x, y, 5*tile, 4*tile, COLOR_GREY);\n"
"        for (int i = 0; i < 20; i++) {\n"
"                int col = i / 4;\n"
"                int row = i % 4;\n"
"\n"
"                int bit = 0xCCCAC & BIT(20-1 - row - 4 * col);\n"
"                fb_fill_rect(fb, x + col*tile + 2, y + row*tile + 2, tile-4, tile-4,\n"
"                                bit? COLOR_WHITE:COLOR_BLACK);\n"
"        }\n"
"}\n";

static void endcard_init(void *ctx) {
	struct endcard *ec = ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Ende");

	font_draw_main(font_default, fb, 0, "- thanks for listening");
	font_draw_main(font_default, fb, 1, "                 -- jn");
	//draw_cccac(fb, 500, 40, 50);
	draw_cccac(fb, MAIN_TEXT_X + 20, 200, 50);
	font_draw_text_window(font_default, fb, 400, 40 + 60 * 4 + 20, "slide_endcard.c", draw_cccac_source);

	fb_present(fb);

	scale_down(ec, fb_bootsplash);
	ec->px = FB_WIDTH;  // scroll in from the right margin
	ec->px += 30 * 60;  // ... after a minute (60s at 30fps)
}

//       - gruetzkopf 🦈 Ghidra team 🦈 kimiko festival ...
const char *const shoutouts[] = {
	"shoutouts to:  ",
	"Alyssa Rosenzweig",
	"Atomic Shrimp",
	"whitequark",
	"LOOK MUM NO COMPUTER",
	"marcan",
	"CyReVolt",
	"foone",
	"HAINBACH",
	"Slavfox",
	"Kimiko Festival",
	"FX",
	"Tech Ingredients",
	"Strange Parts",
	"archive.org",
	"Ghidra developers",
	"Montage LZ",
	"zerforschung",
	"gruetzkopf",
	"Rich Felker",
	"all hackers all over the world"
};

static void endcard_render_shoutouts(struct endcard *ec, FB fb)
{
	int y = 500;

	//putstr(": "); put_hex32(ec->px); putstr(" (-"); put_hex32(-ec->px); putstr(") ");
	//putstr(" "); put_hex32(ec->name_index); putchar('\n');

	// First unwrite the old text, then shift by one pixel and write the new text
	for (int pass = 0; pass < 2; pass++) {
		int x = ec->px;

		for (size_t i = ec->name_index; i < ARRAY_LENGTH(shoutouts) && x < FB_WIDTH; i++) {
			const char *name = shoutouts[i];
			int width, height;

			font_measure(font_default, &width, &height, 2, name);
			font_draw(font_default, fb, x, y, 2, pass? COLOR_GREY:COLOR_WHITE, TRANSPARENT, name);
			x += width;

			if (i != 0 && i < ARRAY_LENGTH(shoutouts) - 1) {
				endcard_logo_present(ec, fb, x, y - 26);
				x += ENDCARD_WIDTH;
				width += ENDCARD_WIDTH;
			}

			if (pass == 1 && i == ec->name_index && x <= 0) {
				ec->name_index++;
				ec->px += width;
			}
		}

		if (pass == 0)
			ec->px--;
	}
}

static void endcard_update(void *ctx)
{
	struct endcard *ec = ctx;

	FB fb = fb_get_active();

	endcard_render_shoutouts(ec, fb);

	fb_present(fb);
}

static const struct slide slide_endcard = {
	.init = endcard_init,
	.update = endcard_update,
	.ctx_size = sizeof(struct endcard)
};
