/* SPDX-License-Identifier: MIT */

SECTIONS {
	. = 0x80010000;

	.text : {
		*(.text*);
	}

	.rodata : {
		*(.rodata*);
		*(.data.rel.ro*);
	}

	.bss : {
		_bss_start = .;
		*(.bss*);
		_bss_end = .;
	}
}
