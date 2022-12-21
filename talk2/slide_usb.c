static const char usb_title[] = "microcom -s 115200 /dev/ttyUSB0";
static const char usb_text[] =
"[   11.454938] usb 2-1: new full-speed USB device number 2 using ehci-platform\n"
"[   27.574937] usb 2-1: device not accepting address 2, error -145\n"
"[   27.702946] usb 2-1: new full-speed USB device number 3 using ehci-platform\n"
"[   33.638945] usb 2-1: device descriptor read/64, error -145\n"
"[   44.134938] usb 2-1: device descriptor read/64, error -145\n"
"[   44.243067] usb usb2-port1: attempt power cycle\n"
"[   44.446949] usb 2-1: new full-speed USB device number 4 using ehci-platform\n"
"[   55.222941] usb 2-1: device not accepting address 4, error -145\n"
"[   55.350946] usb 2-1: new full-speed USB device number 5 using ehci-platform\n"
"[   69.606992] usb 2-1: device descriptor read/8, error -145\n"
"[   74.982990] usb 2-1: device descriptor read/8, error -145\n"
"[   75.091051] usb usb2-port1: unable to enumerate USB device\n";

static void usb_init(void *ctx, int step) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#5)");
	font_draw_main(font_default, fb, 0, "- standard USB controller (EHCI)");
	if (step >= 1) {
		font_draw_main(font_default, fb, 1, "- but it doesn't work");
		font_draw_main(font_default, fb, 2, "- devices are detected, enumeration times out");
		font_draw_text_window(font_default, fb, MAIN_TEXT_X + 50, MAIN_TEXT_Y + 200, usb_title, usb_text);
	}
	if (step >= 2)
		font_draw_main(font_default, fb, 3, "- help wanted!");

	fb_present(fb);
}

static void usb0_init(void *ctx) { usb_init(ctx, 0); }
static void usb1_init(void *ctx) { usb_init(ctx, 1); }
static void usb2_init(void *ctx) { usb_init(ctx, 2); }

static const struct slide slide_usb0 = { .init = usb0_init, };
static const struct slide slide_usb1 = { .init = usb1_init, };
static const struct slide slide_usb2 = { .init = usb2_init, };
