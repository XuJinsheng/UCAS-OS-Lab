/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */


/* maybe you need another stack for booting? */
.equ boot_stack_top_base, 0x50202000

.section ".bootkernel_entry","ax"
.globl _boot
.align 2
_boot:
	/*
	 * this is the entrypoint of the kernel. 
	 * Don't clear BSS here
	 */

	/* Mask all interrupts */
	csrw sie, zero
	csrw sip, zero

	la sp, boot_stack_top_base

	csrr a0, mhartid 

	// this will never return
	call boot_kernel


loop:
	wfi
	j loop

