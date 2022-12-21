const char zboot_code[] =
"#include <linux/io.h>\n"
"\n"
"#include <asm/setup.h>\n"
"\n"
"#define UART_BASE      ((void __iomem *)KSEG1ADDR(0xbf540000))\n"
"#define UART_TXLVL     (UART_BASE + 0x0010)\n"
"#define UART_TXDATA    (UART_BASE + 0x0100)\n"
"\n"
"#define TXLVL_MAX 64\n"
"\n"
"void putc(char c)\n"
"{\n"
"       while (readw(UART_TXLVL) >= TXLVL_MAX)\n"
"               ;\n"
"       writew(c, UART_TXDATA);\n"
"}\n";

const char zboot_output[] =
"zimage at:     8100D370 810DBA5B\n"
"Uncompressing Linux at load address 80200000\n"
"Copy device tree to address  80464620\n"
"Now, booting the kernel...\n"
"[    0.000000] Linux version 6.1.0-rc5-00005-gf43816e9416e-dirty (jn@probook) (mips-\n"
"linux-gnu-gcc (Debian 11.2.0-18) 11.2.0, GNU ld (GNU Binutils for Debian) 2.39) #46 \n"
"Sat Nov 19 17:16:31 CET 2022\n"
"[    0.000000] printk: bootconsole [early0] enabled\n"
"[    0.000000] CPU0 revision is: 00019655 (MIPS 24KEc)\n"
"[    0.000000] OF: fdt: No chosen node found, continuing without\n"
"[    0.000000] MIPS: machine is HS1168-8001-02B\n"
"[    0.000000] Primary instruction cache 16kB, VIPT, 4-way, linesize 32 bytes.\n"
"[    0.000000] Primary data cache 16kB, 4-way, VIPT, no aliases, linesize 32 bytes\n"
"[    0.000000] Zone ranges:\n"
"[    0.000000]   Normal   [mem 0x0000000000000000-0x0000000003ffffff]\n"
"[    0.000000] Movable zone start for each node\n"
"[    0.000000] Early memory node ranges\n"
"[    0.000000]   node   0: [mem 0x0000000000000000-0x0000000003ffffff]\n"
"[    0.000000] Initmem setup node 0 [mem 0x0000000000000000-0x0000000003ffffff]\n"
"[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 16256\n"
"[    0.000000] Kernel command line:\n"
"[    0.000000] Dentry cache hash table entries: 8192 (order: 3, 32768 bytes, linear)\n"
"[    0.000000] Inode-cache hash table entries: 4096 (order: 2, 16384 bytes, linear)\n"
"[    0.000000] Writing ErrCtl register=00067c09\n"
"[    0.000000] Readback ErrCtl register=00067c09\n";


static void zboot_init(void *ctx) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	// Text
	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#1)");
	font_draw_main(font_default, fb, 0, "- zboot debugging and early printk");

	font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 50,
			"arch/mips/boot/compressed/uart-montage.c", zboot_code);
	font_draw_text_window(font_default, fb, MAIN_TEXT_X + 400, MAIN_TEXT_Y + 50,
			"microcom -s 115200 /dev/ttyUSB0", zboot_output);

	fb_present(fb);
}

static const struct slide slide_zboot = { .init = zboot_init, };
