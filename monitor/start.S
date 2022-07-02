#include <regdef.h>

	.text
entry:
	# Set stack pointer
	lui	sp, 0x8001

	lui	t0, 0xbf54
	li	t1, 'A'
	sh	t1, 0x100(t0)
	li	t1, '\r'
	sh	t1, 0x100(t0)
	li	t1, '\n'
	sh	t1, 0x100(t0)

	bal	main

	# If main returns, print 'z' and hang.
	lui	t0, 0xbf54
	li	t1, 'z'
	sh	t1, 0x100(t0)
loop:
	b	loop


.global synci_line
synci_line:
	synci	0(a0)	# Sync all caches at address
	jr	ra

.global do_call
do_call:
	# void do_call(uint32_t fn, uint32_t a1, uint32_t a2, uint32_t a3);
	move	t9, a0
	move	a0, a1
	move	a1, a2
	move	a2, a3
	sync
	jr.hb	t9
