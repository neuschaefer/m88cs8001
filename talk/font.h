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

struct font {
	uint8_t *bitmaps;
	struct chardef *chars;
	struct {
		int8_t w, h, xoff, yoff;
	} bbx_bias;
	uint16_t first_char, num_chars;
};
