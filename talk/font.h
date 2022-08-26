// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>

struct chardef {
	uint16_t bm_off:12;
	uint16_t bm_len:4;
	struct {
		uint8_t w:4, h:4, xoff:4, yoff:4;
	} bbx;
};

struct bbx {
	int8_t w, h, xoff, yoff;
};

struct remap_entry {
	uint32_t codepoint:21;
	uint32_t index:11;
};

struct font {
	const uint8_t *bitmaps;
	const struct chardef *chars;
	struct bbx bbx_bias;
	uint16_t first_char, num_chars;
	const struct remap_entry *remap_table;
	uint16_t remap_table_len;
	struct bbx font_bbx;
	int8_t norm_space;
};
