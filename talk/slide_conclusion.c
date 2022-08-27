static void conclusion_init(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Fazit");
	font_draw_main(font_default, fb, 0, "- Es gibt Welten zu entdecken!");
	font_draw_main(font_default, fb, 1, "- diese Welt hat 28€ gekostet");

	fb_present(fb);
}

static const struct slide slide_conclusion = { .init = conclusion_init, };
