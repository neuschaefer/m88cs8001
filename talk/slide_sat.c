static void sat_init(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Sat receiver");
	font_draw_main(font_default, fb, 0, "- Antenna socket");
	font_draw_main(font_default, fb, 1, "- HDMI, SCART");
	font_draw_main(font_default, fb, 2, "- USB");
	font_draw_main(font_default, fb, 3, "- a few buttons");
	font_draw_main(font_default, fb, 4, "- 7 segment display");
	font_draw_main(font_default, fb, 5, "- RS-232 (serial)");
	font_draw_main(font_default, fb, 7, "- 4 MB flash");
	font_draw_main(font_default, fb, 8, "- 2x MIPS CPU");

	fb_present(fb);
}

static const struct slide slide_sat = { .init = sat_init, };
