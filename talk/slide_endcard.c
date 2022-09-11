// SPDX-License-Identifier: MIT

struct endcard {
};

static void endcard_init(void *ctx) {
	(void)ctx;
}

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

static void endcard_draw(void *ctx)
{
	(void)ctx;
//       - gruetzkopf 🦈 Ghidra team 🦈 kimiko festival ...
}

static const struct slide slide_endcard = {
	.init = endcard_init,
	.draw = endcard_draw,
	.ctx_size = sizeof(struct endcard)
};
