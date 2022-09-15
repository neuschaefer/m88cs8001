static void conclusion_init(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Conclusion");
	font_draw_main(font_default, fb, 0, "- There are worlds to discover");
	font_draw_main(font_default, fb, 1, "- This world cost 28€");

	fb_present(fb);
}

static const struct slide slide_conclusion = { .init = conclusion_init, };
