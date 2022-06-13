	.text
entry:
	.word	0b001000 << 26 | 0 << 21 | 9 << 16 | 'A'    # li  t1, 'A'
	.word	0b001111 << 26 | 0 << 21 | 8 << 16 | 0xbf54 # lui t0, 0xbf54
loop:	.word	0b101001 << 26 | 8 << 21 | 9 << 16 | 0x0100 # sh  t1, 0x100(t0)
	b	loop
	nop
