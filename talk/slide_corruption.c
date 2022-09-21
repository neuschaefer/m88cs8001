static const char corruption_commands[] =
">>> FB = 0xa21e5840\n"
">>> l.copy8(FB + 100 * 1920, FB + 200 * 1920, 400 * 1920)\n";

static void corruption0_init(void *ctx)
{
	(void)ctx;

	// Show something, quickly
	FB fb = fb_get_free();
	fb_copy(fb, fb_bootsplash);
	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Framebuffer?");
	font_draw_text_window(font_default, fb, 550, 150, "python3 -i interact.py", corruption_commands);
	fb_present(fb);
}

static void corruption1_init(void *ctx)
{
	corruption0_init(ctx);

	// Now, do the corruption
	uint8_t *chunk = arena_alloc(8 * MiB);
	memcpy(chunk, MEM_C(0xa1f00000), 8 * MiB);
	FB fb = {
		.luma   = MEM_DMA(fb_bootsplash.luma)   - 0x01f00000 + chunk,
		.chroma = MEM_DMA(fb_bootsplash.chroma) - 0x01f00000 + chunk,
	};
	//put_hex32((uint32_t)fb_bootsplash.luma); putchar(' ');
	//put_hex32((uint32_t)fb_bootsplash.chroma); putchar('\n');

	memcpy(MEM_C(0xa21e5840 - 0xa1f00000 + chunk + 100 * 1920),
	       MEM_C(0xa21e5840 - 0xa1f00000 + chunk + 200 * 1920), 400 * 1920);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Framebuffer?");
	font_draw_text_window(font_default, fb, 550, 150, "python3 -i interact.py", corruption_commands);

	fb_present(fb);
}

static const struct slide slide_corruption0 = { .init = corruption0_init, };
static const struct slide slide_corruption1 = { .init = corruption1_init, };


struct lumafound {
	int i, j;
};

static void lumafound_init(void *ctx)
{
	//struct lumafound *l = ctx;
	(void)ctx;

	FB fb = fb_get_free();
	fb_copy(fb, fb_bootsplash);
	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Framebuffer!");

	static const char commands[] =
	">>> for i in range(256): write8(0xa20455d4 + 1920 * i, range(256))";
	font_draw_text_window(font_default, fb, 550, 150, "python3 -i interact.py", commands);

	fb_present(fb);
}

static void lumafound_update(void *ctx)
{
	struct lumafound *l = ctx;

	FB fb = fb_get_active();

	for (int i = 0; i < 20; i++) {
		if (l->i < 256) {
			if (l->j < 256) {
				luma_set(fb.luma, 200 + l->j, 500 + l->i, YCbCr(l->j, 0, 0));
				l->j++;
			} else {
				l->j = 0;
				l->i++;
			}
		}
	}

	fb_present(fb);
}

static const struct slide slide_lumafound = {
	.init = lumafound_init,
	.update = lumafound_update,
	.ctx_size = sizeof(struct lumafound)
};
