static const char scream_asm[] =
"#include <regdef.h>\n"
"\n"
"        .text\n"
"entry:\n"
"        li      t1, 'A'\n"
"        lui     t0, 0xbf54\n"
"loop:   sh      t1, 0x100(t0)\n"
"        b       loop\n"
"        nop\n";

static const char scream_output[] =
"*****************************************\n"
"**  Board: mips CPU: sym - MIPS 24KEc\n"
"**  SOC name  : 0x8080\n"
"**  PACKET type : SIP_68S_DDR2\n"
"*****************************************\n"
"DRAM:  20 MiB\n"
"phy_clk = 405, clk=50\n"
"R_SPIN_CH0_BAUD: 40000009\n"
"\n"
"[0x1c 0x3016].AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";

static const char lolmon_help[] =
"> help\n"
"help - Show help output for one or all commands\n"
"echo - Echo a few words\n"
"rb - Read one or more bytes\n"
"rh - Read one or more half-words (16-bit)\n"
"rw - Read one or more words (32-bit)\n"
"wb - Write one or more bytes\n"
"wh - Write one or more half-words (16-bit)\n"
"ww - Write one or more words (32-bit)\n"
"cb - Copy one or more bytes\n"
"ch - Copy one or more half-words (16-bit)\n"
"cw - Copy one or more words (32-bit)\n"
"sync - Synchronize caches\n"
"call - Call a function by address\n"
"> rw 80000000 0x40\n"
"80000000: 08085d9b 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"80000020: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"80000040: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"80000060: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"80000080: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"800000a0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"800000c0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
"800000e0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000\n"
">";



static void codeexec_init(void *ctx, int num)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Code execution");
	font_draw_main(font_default, fb, 0, "- Write my own code into flash");
	font_draw_main(font_default, fb, 1, "  (LZMA compressed)");
	if (num >= 2) {
		font_draw_main(font_default, fb, 2, "- \"Monitor\" for");
		font_draw_main(font_default, fb, 3, "  peek/poke/etc.");
	}

	if (num <= 1)
		font_draw_text_window(font_default, fb, MAIN_TEXT_X + 30, 250, "scream.S", scream_asm);

	if (num == 1)
		font_draw_text_window(font_default, fb, 450, 200, "microcom -s 115200 /dev/ttyUSB0", scream_output);

	if (num == 2)
		font_draw_text_window(font_default, fb, 450, 200, "microcom -s 115200 /dev/ttyUSB0", lolmon_help);

	fb_present(fb);
}

static void codeexec0_init(void *ctx) { codeexec_init(ctx, 0); }
static void codeexec1_init(void *ctx) { codeexec_init(ctx, 1); }
static void codeexec2_init(void *ctx) { codeexec_init(ctx, 2); }

static const struct slide slide_codeexec0 = { .init = codeexec0_init, };
static const struct slide slide_codeexec1 = { .init = codeexec1_init, };
static const struct slide slide_codeexec2 = { .init = codeexec2_init, };
