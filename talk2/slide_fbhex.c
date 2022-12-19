// - 0x1000 byte scan
// - dump of interesting region
// - 1920 byte offset

static char *fmt_hex4(char *buf, uint8_t x)
{
	*buf++ = "0123456789abcdef"[x & 15];
	return buf;
}

static char *fmt_hex8(char *buf, uint8_t x)
{
	buf = fmt_hex4(buf, x >> 4);
	buf = fmt_hex4(buf, x);
	return buf;
}

static char *fmt_hex16(char *buf, uint16_t x)
{
	buf = fmt_hex8(buf, x >> 8);
	buf = fmt_hex8(buf, x);
	return buf;
}

static char *fmt_hex32(char *buf, uint32_t x)
{
	buf = fmt_hex16(buf, x >> 16);
	buf = fmt_hex16(buf, x);
	return buf;
}

static const char memscan_source[] =
"def scan_mem():\n"
"    for i in range(0x80000000,\n"
"                   0x80000000 + 64 * MiB, 0x8000):\n"
"        print(f'{i:x}:  ' + ' '.join(\n"
"              [f'{l.read32(i+o*0x1000):08x}'\n"
"               for o in range(8)]))\n";

static char *render_memscan(uint32_t start, uint32_t end)
{
	char *buf = arena_alloc(20 * KiB);
	char *p = buf;

	for (uint32_t i = start; i < end; i += 0x8000) {
		p = fmt_hex32(p, i);
		*p++ = ':';
		*p++ = ' ';

		for (int o = 0; o < 8; o++) {
			*p++ = ' ';
			p = fmt_hex32(p, read32(i+o*0x1000));
		}
		*p++ = '\n';
	}

	*p++ = '\0';
	return buf;
}

static void fbhex0_init(void *ctx)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	// Text
	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "RAM");
	font_draw_main(font_default, fb, 0, "- Where is my");
	font_draw_main(font_default, fb, 1, "  framebuffer?");

	// Little shark
	struct scaled_image *img = scale_down(fb_bootsplash, 4);
	font_draw_window(font_default, fb, MAIN_TEXT_X + 40, MAIN_TEXT_Y + 80, img->width, img->height,
			COLOR_BLACK, TRANSPARENT, COLOR_BLACK, COLOR_GREY_D0, "What I'm looking for");
	scaled_image_present(img, fb, MAIN_TEXT_X + 40, MAIN_TEXT_Y + 80);

	// Memory scan
	char *memscan = render_memscan(0x82200000, 0x82308000);
	font_draw_text_window(font_default, fb, 400, 110, "memscan()", memscan);

	// Memory scan source code
	font_draw_text_window(font_default, fb, MAIN_TEXT_X + 40, MAIN_TEXT_Y + 260, "interact.py", memscan_source);

	fb_present(fb);
}

static const struct slide slide_fbhex0 = { .init = fbhex0_init, };


static const char fbdump[] =
"82238280: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"822382a0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"822382c0: 80808080 80808080 80808080 80808080 8080807f 76837c81 7b857983 7b857a86\n"
"822382e0: 7a867b85 7a867a86 79867986 7a867986 7a867985 7c867b86 7b867b85 7b867b86\n"
"82238300: 7a857b85 78877886 7b817584 80808080 80808080 80808080 80808080 80808080\n"
"82238320: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82238340: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82238360: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82238380: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"...\n"
"82238a40: 80808080 80808080 80808080 80808080 80808080 75847f80 79847783 7a857985\n"
"82238a60: 7a857b86 7a867a85 7a877986 78867986 7a857986 7d867c85 7c867c86 7c857c84\n"
"82238a80: 7b857b85 78867a85 76837786 80807f80 80808080 80808080 80808080 80808080\n"
"82238aa0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"...\n"
"822391c0: 80808080 80808080 80808080 80808080 80808080 75857d81 79847683 7a857985\n"
"822391e0: 7b867a85 7a867a84 79867986 7a857986 7c857986 7c867c85 7d857c86 7c867c86\n"
"82239200: 79857c85 78867a85 76857686 81807981 80808080 80808080 80808080 80808080\n"
"82239220: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n";

static const char offset_calculations[] =
">>> 0x82238a40 - 0x822382c0\n"
"1920\n"
">>> 0x822391c0 - 0x82238a40\n"
"1920\n";

static void fbhex1_init(void *ctx, int num)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	// Text
	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "RAM");
	font_draw_main(font_default, fb, 0, "- Where is my");
	font_draw_main(font_default, fb, 1, "  framebuffer?");

	// Little shark
	struct scaled_image *img = scale_down(fb_bootsplash, 4);
	font_draw_window(font_default, fb, MAIN_TEXT_X + 40, MAIN_TEXT_Y + 80, img->width, img->height,
			COLOR_BLACK, TRANSPARENT, COLOR_BLACK, COLOR_GREY_D0, "What I'm looking for");
	scaled_image_present(img, fb, MAIN_TEXT_X + 40, MAIN_TEXT_Y + 80);

	// Memory scan
	font_draw_text_window(font_default, fb, 400, MAIN_TEXT_Y, "rw 822095e0 0x10000", fbdump);
	font_draw_text_window(font_default, fb, 400, MAIN_TEXT_Y + 280, "python3", offset_calculations);

	if (num >= 1)
		font_draw_main(font_default, fb, 9, "- 1920 bytes distance -> one raster line!");

	fb_present(fb);
}

static void fbhex1a_init(void *ctx) { fbhex1_init(ctx, 0); }
static void fbhex1b_init(void *ctx) { fbhex1_init(ctx, 1); }

static const struct slide slide_fbhex1a = { .init = fbhex1a_init, };
static const struct slide slide_fbhex1b = { .init = fbhex1b_init, };
