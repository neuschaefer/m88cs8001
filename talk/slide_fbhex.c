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
"def memscan():\n"
"    for i in range(0x80000000, 0x80000000 + 64 * MiB, 0x8000):\n"
"        print(f'{i:x}:  ' + ' '.join([f'{l.read32(i+o*0x1000):08x}' for o in range(8)]))\n";

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
	char *memscan = render_memscan(0x821e0000, 0x822e8000);
	font_draw_text_window(font_default, fb, 400, 110, "memscan()", memscan);

	// Memory scan source code
	font_draw_text_window(font_default, fb, 400, 40, "interact.py", memscan_source);

	fb_present(fb);
}

static const struct slide slide_fbhex0 = { .init = fbhex0_init, };


static const char fbdump[] =
"822095e0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82209600: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82209620: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82209640: 80808080 80808080 80808080 80808080 80818080 80828081 7f828082 7f827f82\n"
"82209660: 7c817c82 7e7e7d7f 807e7f7d 807f807f 807d817c 7e817f7f 7f817e81 82808180\n"
"82209680: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"822096a0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"822096c0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"...\n"
"82209dc0: 80808080 80808080 80808080 80808080 80828081 80838082 81818182 827f8180\n"
"82209de0: 807c7f7e 83778179 84758475 84778476 837a8578 7f80817d 7f827e81 81808081\n"
"82209e00: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"82209e20: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"...\n"
"8220a540: 80808080 80808080 80808080 80808080 7f807f7f 81808080 837d827f 857a847b\n"
"8220a560: 87738576 8b6c896f 8c698c69 8b6a8c69 86768973 807e837a 7e827e81 80817f82\n"
"8220a580: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n"
"8220a5a0: 80808080 80808080 80808080 80808080 80808080 80808080 80808080 80808080\n";

static const char offset_calculations[] =
">>> 0x82209de0 - 0x82209660\n"
"1920\n"
">>> 0x8220a560 - 0x82209de0\n"
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
	font_draw_text_window(font_default, fb, 400, 40, "rw 822095e0 0x10000", fbdump);
	font_draw_text_window(font_default, fb, 400, 320, "python3", offset_calculations);

	if (num >= 1)
		font_draw_main(font_default, fb, 8, "- 1920 bytes distance -> one raster line!");

	fb_present(fb);
}

static void fbhex1a_init(void *ctx) { fbhex1_init(ctx, 0); }
static void fbhex1b_init(void *ctx) { fbhex1_init(ctx, 1); }

static const struct slide slide_fbhex1a = { .init = fbhex1a_init, };
static const struct slide slide_fbhex1b = { .init = fbhex1b_init, };
