/* SPDX-License-Identifier: MIT */

SECTIONS {
	. = 0x9e800000;

	.text : {
		*(.text*);
	}

	.rodata : {
		*(.rodata*);
		*(.data.rel.ro*);
	}
}
