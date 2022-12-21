static void uart_init(void *ctx, int step) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#4)");
	font_draw_main(font_default, fb, 0, "- UART / serial port driver");
	if (step >= 1) {
		font_draw_main(font_default, fb, 1, "- tty subsystem is a bit difficult");
		font_draw_main(font_default, fb, 2, "- 355 lines for basic functionality");
	}

	fb_present(fb);
}

static void uart0_init(void *ctx) { uart_init(ctx, 0); }
static void uart1_init(void *ctx) { uart_init(ctx, 1); }

static const struct slide slide_uart0 = { .init = uart0_init, };
static const struct slide slide_uart1 = { .init = uart1_init, };
