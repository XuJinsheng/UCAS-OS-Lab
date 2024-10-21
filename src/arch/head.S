/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */
#include <arch/CSR.h>
.set KERNEL_STACK, 0xffffffc050500000

.section ".entry_function","ax"
.globl _start
.balign 4
_start:
    // a0 is mhartid, do not modify

	/* Mask all interrupts */
	csrw sie, zero
	csrw sie, zero
	csrw sip, zero
	csrw sip, zero

	li t0, SR_SUM
	csrs sstatus, t0

    li gp,0
    li tp,0


	// set kernel stack = KERNEL_STACK - mhartid * 0x10000(64KB)
	li sp,KERNEL_STACK
	li t0, 0x10000
	mul t0, t0, a0
	sub sp, sp, t0
	j main


loop:
	wfi
	j loop


