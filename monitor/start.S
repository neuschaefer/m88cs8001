#include <regdef.h>

	.text
entry:
	li	t1, 'A'
	lui	t0, 0xbf54
loop:	sh	t1, 0x100(t0)
	b	loop
	nop
