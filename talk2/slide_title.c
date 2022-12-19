// SPDX-License-Identifier: MIT

static void title_init(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();
	//fb_fill(fb, COLOR_WHITE);
	fb_copy(fb, fb_bootsplash);

	font_draw_title(font_default, fb, COLOR_BLACK, COLOR_GREY, " Hacking a\n\n\n   sat receiver");
	fb_present(fb);
}

static const struct slide slide_title = {
	.init = title_init,
};
