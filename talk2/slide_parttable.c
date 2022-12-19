static const char part_table_hexdump[] =
"00010000  2a 5e 5f 5e 2a 44 4d 28  5e 6f 5e 29 00 00 3f 00  |*^_^*DM(^o^)..?.|\n"
"00010010  00 00 00 04 00 00 40 00  15 00 30 00 fc 00 01 00  |......@...0.....|\n"
"00010020  00 04 00 00 56 72 04 00  5e f3 fd 3f 30 30 30 30  |....Vr..^..?0000|\n"
"00010030  30 30 30 31 61 76 5f 63  70 75 00 00 e6 07 01 06  |0001av_cpu......|\n"
"00010040  0a 2a 38 00 00 00 00 00  00 00 00 00 89 00 01 00  |.*8.............|\n"
"00010050  56 76 04 00 54 53 03 00  4e 43 52 43 30 30 30 30  |Vv..TS..NCRC0000|\n"
"00010060  30 30 30 31 69 6d 67 00  00 00 00 00 e6 07 01 06  |0001img.........|\n"
"00010070  0a 29 29 00 00 00 00 00  00 00 00 00 88 00 01 00  |.)).............|\n"
"00010080  aa c9 07 00 6c 42 21 00  4e 43 52 43 30 30 30 30  |....lB!.NCRC0000|\n"
"00010090  30 30 30 31 64 65 6d 6f  00 00 00 00 e6 07 01 06  |0001demo........|\n"
"000100a0  0a 2a 38 00 00 00 00 00  00 00 00 00 af 00 01 00  |.*8.............|\n"
"000100b0  16 0c 29 00 48 00 00 00  4e 43 52 43 30 30 30 30  |..).H...NCRC0000|\n"
"000100c0  30 30 30 31 69 72 31 00  00 00 00 00 e6 07 01 06  |0001ir1.........|\n"
"000100d0  0a 29 29 00 00 00 00 00  00 00 00 00 b1 00 01 00  |.)).............|\n"
"000100e0  5e 0c 29 00 39 00 00 00  4e 43 52 43 30 30 30 30  |^.).9...NCRC0000|\n"
"000100f0  30 30 30 31 69 72 32 00  00 00 00 00 e6 07 01 06  |0001ir2.........|\n"
"00010100  0a 29 29 00 00 00 00 00  00 00 00 00 b0 00 01 00  |.)).............|\n"
"00010110  97 0c 29 00 09 00 00 00  4e 43 52 43 30 30 30 30  |..).....NCRC0000|\n"
"00010120  30 30 30 31 66 70 00 00  00 00 00 00 e6 07 01 06  |0001fp..........|\n"
"00010130  0a 29 29 00 00 00 00 00  00 00 00 00 bb 00 01 00  |.)).............|\n"
"00010140  a0 0c 29 00 10 00 00 00  4e 43 52 43 30 30 30 30  |..).....NCRC0000|\n"
"00010150  30 30 30 31 73 64 72 61  6d 00 00 00 e6 07 01 06  |0001sdram.......|\n"
"00010160  0a 29 29 00 00 00 00 00  00 00 00 00 b4 00 01 00  |.)).............|\n"
"00010170  b0 0c 29 00 60 00 00 00  4e 43 52 43 30 30 30 30  |..).`...NCRC0000|\n"
"00010180  30 30 30 31 6d 69 73 63  00 00 00 00 e6 07 01 06  |0001misc........|\n"
"00010190  0a 29 29 00 00 00 00 00  00 00 00 00 8c 00 01 00  |.)).............|\n"
"000101a0  10 0d 29 00 52 bc 03 00  4e 43 52 43 30 30 30 30  |..).R...NCRC0000|\n"
"000101b0  30 30 30 31 72 65 73 6f  75 72 63 65 e6 07 01 06  |0001resource....|\n"
"000101c0  0a 29 29 00 00 00 00 00  00 00 00 00 93 00 01 00  |.)).............|\n"
"000101d0  62 c9 2c 00 fe 54 00 00  4e 43 52 43 30 30 30 30  |b.,..T..NCRC0000|\n"
"000101e0  30 30 30 31 6c 6f 67 6f  00 00 00 00 e6 07 01 06  |0001logo........|\n"
"000101f0  0a 29 29 00 00 00 00 00  00 00 00 00 7d 00 01 00  |.)).........}...|";

static const char part_table_decoded[] =
"nr. flags    offset   size     crc32    name\n"
" 0  00400000      400    47256 3ffdf35e av_cpu\n"
"CRC is correct!\n"
" 1  00000000    47656    35354 4352434e img\n"
" 2  00000000    7c9aa   21426c 4352434e demo\n"
" 3  00000000   290c16       48 4352434e ir1\n"
" 4  00000000   290c5e       39 4352434e ir2\n"
" 5  00000000   290c97        9 4352434e fp\n"
" 6  00000000   290ca0       10 4352434e sdram\n"
" 7  00000000   290cb0       60 4352434e misc\n"
" 8  00000000   290d10    3bc52 4352434e resource\n"
" 9  00000000   2cc962     54fe 4352434e logo\n"
"10  00000000   2d1e60     1510 4352434e cas\n"
"11  00000000   2d3370     c7c6 4352434e radio\n"
"12  00000000   2dfb36     b2e0 4352434e radio2\n"
"13  00000000   2eae16      945 4352434e music_lo\n"
"14  00000000   2eb75b     6078 4352434e ssdata\n"
"15  00000000   2f17d3     9956 4352434e preset\n"
"16  00000000   2fb129      300 4352434e preset3\n"
"17  00000000   340000    80000 4352434e iwtable\n"
"18  00000000   3c0000    10000 4352434e iwview\n"
"19  00000000   3d0000    10000 4352434e ucaskey";

static const char *render_part_table_extract(void)
{
	char *buf = arena_alloc(4000);
	size_t offset = 0, len;

	static const char *const partitions[] = {
		"av_cpu", "img", "demo", "ir1", "ir2", "fp", "sdram", "misc",
		"resource", "logo", "cas", "radio", "radio2", "music_lo",
		"ssdata", "preset", "preset3", "iwtable", "iwview", "ucaskey"
	};

	for (size_t i = 0; i < ARRAY_LENGTH(partitions); i++) {
		// Extracting partition "av_cpu"
		const char *prefix = "Extracting partition \"";
		const char *name = partitions[i];
		const char *suffix = "\"\n";

		len = strlen(prefix);
		memcpy(buf + offset, prefix, len);
		offset += len;

		len = strlen(name);
		memcpy(buf + offset, name, len);
		offset += len;

		len = strlen(suffix);
		memcpy(buf + offset, suffix, len);
		offset += len;
	}

	buf[offset] = '\0';
	return buf;
}

const char part_file[] =
"av_cpu.bin:   data\n"
"cas.bin:      data\n"
"demo.bin:     LZMA compressed data, non-streamed, size 5675288\n"
"fp.bin:       data\n"
"img.bin:      LZMA compressed data, non-streamed, size 649701\n"
"ir1.bin:      data\n"
"ir2.bin:      data\n"
"iwtable.bin:  data\n"
"iwview.bin:   ISO-8859 text, with very long lines (65536), with no line terminators\n"
"logo.bin:     JPEG image data, JFIF standard 1.02, aspect ratio, density 100x100, segment length 16, progressive, precision 8, 1920x1080, components 3\n"
"misc.bin:     data\n"
"music_lo.bin: GIF image data, version 89a, 200 x 100\n"
"preset3.bin:  data\n"
"preset.bin:   data\n"
"radio2.bin:   MPEG sequence, v2, MP@HL progressive Y'CbCr 4:2:0 video, HD-TV 1920P, 16:9, 25 fps\n"
"radio.bin:    MPEG sequence, v2, MP@H-14 progressive Y'CbCr 4:2:0 video, 25 fps\n"
"resource.bin: LZMA compressed data, non-streamed, size 808792\n"
"sdram.bin:    data\n"
"ssdata.bin:   data\n"
"ucaskey.bin:  data";

static const char demo_asm[] =
"ram:80008000    00600840        mfc0        t0,Status\n"
"ram:80008004    8410087c        ins         t0,zero,0x2,0x1\n"
"ram:80008008    01000924        li          t1,0x1\n"
"ram:8000800c    4408287d        ins         t0,t1,0x1,0x1\n"
"ram:80008010    04c6287d        ins         t0,t1,0x18,0x1\n"
"ram:80008014    00608840        mtc0        t0,Status,0x0\n"
"ram:80008018    c0000000        ehb\n"
"ram:8000801c    31000924        li          t1,0x31\n"
"ram:80008020    54bf0d3c        lui         t5,0xbf54\n"
"ram:80008024    0001ad35        ori         t5,t5,0x100\n"
"ram:80008028    0000a9a5        sh          t1,0x0(t5)\n"
"ram:8000802c    00000000        nop\n"
"ram:80008030    00000000        nop\n"
"ram:80008034    25080000        or          at,zero,zero\n"
"ram:80008038    25100000        or          v0,zero,zero\n"
"ram:8000803c    25180000        or          v1,zero,zero\n"
"ram:80008040    25200000        or          a0,zero,zero\n"
"ram:80008044    25280000        or          a1,zero,zero\n"
"ram:80008048    25300000        or          a2,zero,zero\n"
"ram:8000804c    25380000        or          a3,zero,zero\n"
"ram:80008050    25400000        or          t0,zero,zero\n";



static void part_init(void *ctx, int num)
{
	(void)ctx;

	FB fb = fb_get_free();
	fb_fill(fb, COLOR_WHITE);

	font_draw_headline(font_default, fb, COLOR_BLACK, COLOR_GREY, "hex hex");

	//font_draw_text_window(font_default, "hexdump -C flash.bin", part_table_hexdump);

	int hexdump_x = 380;
	int hexdump_y = HEADLINE_Y;

	if (num < 2)
		font_draw_text_window(font_default, fb, hexdump_x, hexdump_y,
				"hexdump -C flash.bin", part_table_hexdump);

	if (num == 1 || num == 2)
		font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y,
				"parse-parttable.py --list flash.bin", part_table_decoded);

	if (num == 2)
		font_draw_text_window(font_default, fb, hexdump_x, MAIN_TEXT_Y,
				"parse-parttable.py --extract flash.bin", render_part_table_extract());

	if (num >= 3) {
		font_draw_text_window(font_default, fb, MAIN_TEXT_X, MAIN_TEXT_Y,
				"file *", part_file);
	}

	if (num >= 4) {
		struct scaled_image *img = scale_down(fb_bootsplash, 3);
		font_draw_window(font_default, fb, 550, 30, img->width, img->height,
				COLOR_BLACK, TRANSPARENT, COLOR_BLACK, COLOR_GREY_D0, "logo");
		scaled_image_present(img, fb, 550, 30);
	}

	if (num >= 5)
		font_draw_text_window(font_default, fb, 550, 260, "demo", demo_asm);

	fb_present(fb);
}

static void part0_init(void *ctx) { part_init(ctx, 0); }
static void part1_init(void *ctx) { part_init(ctx, 1); }
static void part2_init(void *ctx) { part_init(ctx, 2); }
static void part3_init(void *ctx) { part_init(ctx, 3); }
static void part4_init(void *ctx) { part_init(ctx, 4); }
static void part5_init(void *ctx) { part_init(ctx, 5); }

static const struct slide slide_part0 = { .init = part0_init, };
static const struct slide slide_part1 = { .init = part1_init, };
static const struct slide slide_part2 = { .init = part2_init, };
static const struct slide slide_part3 = { .init = part3_init, };
static const struct slide slide_part4 = { .init = part4_init, };
static const struct slide slide_part5 = { .init = part5_init, };
