// SPDX-License-Identifier: MIT


struct endcard {
	// Scaled version of the boot splash, for use between names
	struct scaled_image *logo;
	size_t name_index;
	int px;
};

static void draw_cccac(FB fb, int x, int y, int tile) {
	fb_fill_rect(fb, x-2, y-2, 5*tile+4, 4*tile+4, COLOR_BLACK_20);
	for (int i = 0; i < 20; i++) {
		int col = i / 4;
		int row = i % 4;

		int bit = 0xCCCAC & BIT(20-1 - row - 4 * col);
		fb_fill_rect(fb, x + col*tile + 2, y + row*tile + 2, tile-4, tile-4,
				bit? COLOR_WHITE:COLOR_BLACK);
	}
	font_draw(font_default, fb, x+10, y+3*tile-8, 5, COLOR_WHITE, TRANSPARENT, "CCCAC");
}

static const char draw_cccac_source[] =
"static void draw_cccac(FB fb, int x, int y, int tile) {\n"
"        fb_fill_rect(fb, x-2, y-2, 5*tile+4, 4*tile+4, COLOR_BLACK_20);\n"
"        for (int i = 0; i < 20; i++) {\n"
"                int col = i / 4;\n"
"                int row = i % 4;\n"
"\n"
"                #define BIT(x) (1UL << (x))\n"
"                int bit = 0xCCCAC & BIT(20-1 - row - 4 * col);\n"
"                fb_fill_rect(fb, x + col*tile + 2, y + row*tile + 2, tile-4, tile-4,\n"
"                                bit? COLOR_WHITE:COLOR_BLACK);\n"
"        }\n"
"        font_draw(font_default, fb, x+10, y+3*tile-8, 5, COLOR_WHITE, TRANSPARENT, \"CCCAC\");\n"
"}\n";

static void endcard_init(void *ctx) {
	struct endcard *ec = ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "The End");

	font_draw_main(font_default, fb, 0, "- Thank you for listening!");
	font_draw_main(font_default, fb, 1, "- GitHub: neuschaefer/m88cs8001");
	font_draw_main(font_default, fb, 2, "- Questions? :)");

	// CCCAC demo
	//draw_cccac(fb, 606, 20, 60);
	draw_cccac(fb, MAIN_TEXT_X, 240, 58);
	font_draw_text_window(font_default, fb, 380, 300, "slide_endcard.c", draw_cccac_source);

	// Font credit
	font_draw(font_default, fb, 4, 530, 2, COLOR_GREY, TRANSPARENT, "Font: Cozette by Slavfox");

	fb_present(fb);

	ec->logo = scale_down(fb_bootsplash, 15);
	ec->px = FB_WIDTH;  // scroll in from the right margin
	ec->px += 30 * 60;  // ... after a minute (60s at 30fps)
}

//       - gruetzkopf 🦈 Ghidra team 🦈 kimiko festival ...
const char *const shoutouts[] = {
	"shoutouts to:  ",
	"Haecksen",
	"r3s",
	"das-labor.org",
	"archive.org",
	"Ghidra developers",
	"Montage LZ",
	"zerforschung",
	"Kimiko Festival",
	"Alyssa Rosenzweig",
	"Atomic Shrimp",
	"whitequark",
	"LOOK MUM NO COMPUTER",
	"marcan",
	"CyReVolt",
	"foone",
	"0xide",
	"HAINBACH",
	"FX",
	"Tech Ingredients",
	"Strange Parts",
	"gruetzkopf",
	"Mimoja",
	"Rich Felker",
	"Slavfox",
	"OSFC",
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
				scaled_image_present(ec->logo, fb, x, y - 26);
				x += ec->logo->width;
				width += ec->logo->width;
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
