/* SPDX-License-Identifier: MIT */

SECTIONS {
	. = 0x80008000;

	.text : {
		*(.text*);
	}

	.rodata : {
		*(.rodata*);
		*(.data.rel.ro*);
	}
}
