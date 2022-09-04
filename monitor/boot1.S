#include <regdef.h>

#define TAGLO $28

.org	0x400

entry:
	li	t1, 'A'
	lui	t0, 0xbf54
	sh	t1, 0x100(t0)

	# Set stack pointer
	li	sp, 0x9e804000

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
