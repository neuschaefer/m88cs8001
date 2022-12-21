const char kconfig_code[] =
"config MACH_MONTAGE\n"
"       bool \"Montage Technologies SoC support\"\n"
"       select SYS_SUPPORTS_32BIT_KERNEL\n"
"       select SYS_SUPPORTS_LITTLE_ENDIAN\n"
"       select SYS_SUPPORTS_MIPS16\n"
"       select SYS_SUPPORTS_ZBOOT\n"
"       select SYS_HAS_EARLY_PRINTK\n"
"       select SYS_HAS_CPU_MIPS32_R2\n"
"       select USE_OF\n"
"       help\n"
"         Enable support for Montage Technologies SoCs\n";

const char platform_title[] = "arch/mips/montage/Platform";
const char platform_code[] =
"cflags-$(CONFIG_MACH_MONTAGE)  += -I$(srctree)/arch/mips/include/asm/mach-montage\n"
"load-$(CONFIG_MACH_MONTAGE)    = 0x80200000\n";

const char makefile_title[] = "arch/mips/montage/Makefile";
const char makefile_code[] =
"obj-y          += irq.o init.o\n"
"obj-$(CONFIG_EARLY_PRINTK) += early_printk.o\n";

const char montage_code[] =
"#include <linux/console.h>\n"
"\n"
"#include <asm/bootinfo.h>\n"
"#include <asm/prom.h>\n"
"\n"
"const char *get_system_type(void)\n"
"{\n"
"       return \"Montage LZ SoC\";\n"
"}\n"
"\n"
"void __init plat_mem_setup(void)\n"
"{\n"
"       void *dtb;\n"
"\n"
"       dtb = get_fdt();\n"
"       if (dtb == NULL)\n"
"               panic(\"no dtb found\");\n"
"\n"
"       __dt_setup_arch(dtb);\n"
"}\n"
"\n"
"...\n";


static void platform_init(void *ctx) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#2)");
	font_draw_main(font_default, fb, 0, "- initial platform support");

	font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 50,
			"arch/mips/Kconfig", kconfig_code);

	font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 240,
			makefile_title, makefile_code);
	font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 310,
			platform_title, platform_code);

	font_draw_text_window(font_default, fb, MAIN_TEXT_X + 600, MAIN_TEXT_Y + 50,
			"arch/mips/montage/init.c", montage_code);

	fb_present(fb);
}

static const struct slide slide_platform = { .init = platform_init, };
