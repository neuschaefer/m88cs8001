const char irq_title[] = "arch/mips/montage/irq.c";
const char irq_code[] =
"#define MONTAGE_MODE(x)        (0x00 + (x) * 4)\n"
"#define MONTAGE_MASK(x)        (0x60 + (x) * 4)\n"
"#define MONTAGE_CAUSE          0x68\n"
"#define MONTAGE_ACK            0x6c\n"
"\n"
"static void montage_intc_irq_unmask(struct irq_data *d) {\n"
"       int hwirq = irqd_to_hwirq(d);\n"
"       int index = hwirq / 32;\n"
"       int shift = hwirq % 32;\n"
"\n"
"       u32 val = readl(intc->regs + MONTAGE_MASK(index));\n"
"       val &= ~BIT(shift);\n"
"       writel(val, intc->regs + MONTAGE_MASK(index));\n"
"}\n"
"\n"
"static void montage_intc_ack(void) {\n"
"       writel(1, intc->regs + MONTAGE_ACK);\n"
"       while (readl(intc->regs + MONTAGE_ACK) != 0)\n"
"               cpu_relax();\n"
"}\n"
"\n"
"static void montage_intc_irq_handler(struct irq_desc *desc) {\n"
"       unsigned int hwirq = readl(intc->regs + MONTAGE_CAUSE);\n"
"       generic_handle_domain_irq(intc->domain, hwirq);\n"
"       montage_intc_ack();\n"
"}\n";

const char irqdts_title[] = "arch/mips/boot/dts/montage/m88cs8001b.dtsi";
const char irqdts_code[] =
"/ {\n"
"        compatible = \"montage,m88cs8001b\";\n"
"        #address-cells = <1>;\n"
"        #size-cells = <1>;\n"
"\n"
"        cpuintc: interrupt-controller {\n"
"                #address-cells = <0>;\n"
"                #interrupt-cells = <1>;\n"
"                interrupt-controller;\n"
"                compatible = \"mti,cpu-interrupt-controller\";\n"
"        };\n"
"\n"
"        soc {\n"
"                compatible = \"simple-bus\";\n"
"                interrupt-parent = <&intc>;\n"
"\n"
"                intc: interrupt-controller@1f100000 {\n"
"                        compatible = \"montage,m88cs8001b-intc\";\n"
"                        reg = <0x1f100000 0x80>;\n"
"                        interrupt-controller;\n"
"                        #interrupt-cells = <1>;\n"
"                        interrupt-parent = <&cpuintc>;\n"
"                        interrupts = <4>;\n"
"                };\n"
"        };\n"
"};\n";

static void irq_init(void *ctx) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#3)");
	font_draw_main(font_default, fb, 0, "- Interrupt controller support");

	font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 50,
			irq_title, irq_code);

	font_draw_text_window(font_default, fb, MAIN_TEXT_X + 400, MAIN_TEXT_Y + 50,
			irqdts_title, irqdts_code);

	fb_present(fb);
}


const char manual_title[] = "Programming the MIPS32® 24K® Core Family, Revision 04.63";
const char manual_text[] =
"The features for handling interrupts include:\n"
"\n"
"* Vectored Interrupt (VI) mode offers multiple entry\n"
"  points (one for each of the interrupt sources),\n"
"  instead of the single general exception entry point.\n";


const char veic_title[] = "arch/mips/Kconfig";
const char veic_code[] =
"config MACH_MONTAGE\n"
"        bool \"Montage Technologies SoC support\"\n"
"        select SYS_SUPPORTS_32BIT_KERNEL\n"
"        select SYS_SUPPORTS_LITTLE_ENDIAN\n"
"        select SYS_SUPPORTS_MIPS16\n"
"        select SYS_SUPPORTS_ZBOOT\n"
"        select SYS_HAS_EARLY_PRINTK\n"
"        select SYS_HAS_CPU_MIPS32_R2\n"
"        select CPU_MIPSR2_IRQ_VI   # Vectored Interrupt mode\n"
"        select CPU_MIPSR2_IRQ_EI   # Extended Interrupt mode\n"
"        ...";


static void veic_init(void *ctx, int step) {
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "Porting Linux (#3.75)");
	font_draw_main(font_default, fb, 0, "- Linux hangs on local_irq_enable()");

	if (step >= 1) {
		font_draw_main(font_default, fb, 1, "- need to use \"Vectored Interrupt\" mode");

		font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y + 100,
				manual_title, manual_text);

		font_draw_text_window(font_default, fb, MAIN_TEXT_X + 400, MAIN_TEXT_Y + 100,
				veic_title, veic_code);
	}

	fb_present(fb);
}

static void veic0_init(void *ctx) { veic_init(ctx, 0); }
static void veic1_init(void *ctx) { veic_init(ctx, 1); }

static const struct slide slide_irq = { .init = irq_init, };
static const struct slide slide_veic0 = { .init = veic0_init, };
static const struct slide slide_veic1 = { .init = veic1_init, };
