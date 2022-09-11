// SPDX-License-Identifier: MIT

struct endcard {
};

static void endcard_init(void *ctx) {
	(void)ctx;
	for (int i = 0; i < 2; i++) {
		FB fb = fb_get_free();
		fb_fill(fb, COLOR_WHITE);

		font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Ende");
		fb_present(fb);
	}
}

//       - gruetzkopf 🦈 Ghidra team 🦈 kimiko festival ...
const char *const shoutouts[] = {
	"Alyssa Rosenzweig",
	"Atomic Shrimp",
	"whitequark",
	"LOOK MUM NO COMPUTER",
	"marcan",
	"CyReVolt",
	"cyrozap",
	"foone",
	"HAINBACH",
	"Slavfox",
	"Kimiko Festival",
	"Tech Ingredients",
	"Strange Parts",
	"archive.org",
	"Ghidra developers",
	"zerforschung",
	"gruetzkopf",
	"Rich Felker",
	"all hackers all over the world"
};

static void endcard_update(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();

	fb_present(fb);
}

static const struct slide slide_endcard = {
	.init = endcard_init,
	.update = endcard_update,
	.ctx_size = sizeof(struct endcard)
};
