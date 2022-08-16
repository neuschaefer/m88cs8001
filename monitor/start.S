#include <regdef.h>

	.text
entry:
	# Set stack pointer
	lui	sp, 0x8001

	bal	here
here:

	li	t2, 0x8000800c
	beq	ra, t2, new_world

	lui	t0, 0xbf54
	li	t1, 'R'
	sh	t1, 0x100(t0)
	li	t1, '\r'
	sh	t1, 0x100(t0)
	li	t1, '\n'
	sh	t1, 0x100(t0)

	# Relocate lolmon to its designated home at 0x80008000
	# - copy
	addiu	a0, ra, -0xc		# source address
	addiu	a1, t2, -0xc		# destination address
	li	a2, 8192		# size in bytes
copy_loop:
	lw	t0, 0x00(a0)
	lw	t1, 0x04(a0)
	lw	t2, 0x08(a0)
	lw	t3, 0x0c(a0)
	lw	t4, 0x10(a0)
	lw	t5, 0x14(a0)
	lw	t6, 0x18(a0)
	lw	t7, 0x1c(a0)
	sw	t0, 0x00(a1)
	sw	t1, 0x04(a1)
	sw	t2, 0x08(a1)
	sw	t3, 0x0c(a1)
	sw	t4, 0x10(a1)
	sw	t5, 0x14(a1)
	sw	t6, 0x18(a1)
	sw	t7, 0x1c(a1)
	synci	0(a1)			# Sync all caches at destination address
	addiu	a0, a0,  0x20
	addiu	a1, a1,  0x20
	addiu	a2, a2, -0x20
	bnez	a2, copy_loop

	# - flush caches, sync
	sync
	lui	t9, %hi(new_world)
	addiu	t9, %lo(new_world)
	jr.hb	t9

new_world:
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
