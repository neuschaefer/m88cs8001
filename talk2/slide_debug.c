static const char uartops_title[] = "drivers/tty/serial/8250/8250_port.c";
static const char uartops_code[] =
"static const struct uart_ops serial8250_pops = {\n"
"        ...\n"
"#ifdef CONFIG_CONSOLE_POLL\n"
"        .poll_get_char = serial8250_get_poll_char,\n"
"        .poll_put_char = serial8250_put_poll_char,\n"
"#endif\n"
"};\n";


static const char sysrq_title[] = "drivers/tty/serial/8250/8250_port.c";
static const char sysrq_code[] =
"void serial8250_read_char(struct uart_8250_port *up, u16 lsr)\n"
"{\n"
"        struct uart_port *port = &up->port;\n"
"        unsigned char ch;\n"
"        char flag = TTY_NORMAL;\n"
"\n"
"        if (likely(lsr & UART_LSR_DR))\n"
"                ch = serial_in(up, UART_RX);\n"
"        ...\n"
"        if (uart_prepare_sysrq_char(port, ch))\n"
"                return;\n"
"\n"
"        uart_insert_char(port, lsr, UART_LSR_OE, ch, flag);\n"
"}\n"
"EXPORT_SYMBOL_GPL(serial8250_read_char);\n";


static void debug_init(void *ctx, int step) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Debugging Linux");
	font_draw_main(font_default, fb, 0, "- Linux can take a long time to load");
	if (step >= 1)
		font_draw_main(font_default, fb, 1, "- Use kgdb and your favorite debugger");
	if (step >= 2) {
		font_draw_main(font_default, fb, 2, "- Requires some features in our UART driver");

		font_draw_text_window(font_default, fb, MAIN_TEXT_X + 50, MAIN_TEXT_Y + 150,
				uartops_title, uartops_code);
		font_draw_text_window(font_default, fb, MAIN_TEXT_X + 400, MAIN_TEXT_Y + 150,
				sysrq_title, sysrq_code);
	}

	fb_present(fb);
}

static void debug0_init(void *ctx) { debug_init(ctx, 0); }
static void debug1_init(void *ctx) { debug_init(ctx, 1); }
static void debug2_init(void *ctx) { debug_init(ctx, 2); }

static const struct slide slide_debug0 = { .init = debug0_init, };
static const struct slide slide_debug1 = { .init = debug1_init, };
static const struct slide slide_debug2 = { .init = debug2_init, };
