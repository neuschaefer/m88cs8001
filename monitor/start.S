#include <regdef.h>

	.text
entry:
	# Set stack pointer
	lui	sp, 0x8001

	lui	t0, 0xbf54
	li	t1, 'A'
	sh	t1, 0x100(t0)

	bal	main

	// If main returns, print 'z' and hang.
	lui	t0, 0xbf54
	li	t1, 'z'
	sh	t1, 0x100(t0)
loop:
	b	loop
