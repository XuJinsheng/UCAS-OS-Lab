/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
	/* Mask all interrupts */
	csrw sie, zero
	csrw sie, zero

    li gp,0
    li tp,0

	/* TODO: [p1-task2] clear BSS for flat non-ELF images */
	la t0,__bss_start
	la t1,__BSS_END__
	beq t0,t1,2$
1$:
	// make sure the bss is 8-byte aligned
	sd zero,0(t0)
	addi t0,t0,8
	blt t0,t1,1$
2$:

	/* TODO: [p1-task2] setup C environment */
	la sp,KERNEL_STACK
	j main

loop:
	wfi
	j loop

END(_start)
