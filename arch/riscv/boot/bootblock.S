#include <asm/biosdef.h>

// os size location (os_size could be sectors [p1-task3] or bytes [p1-task4])
.equ os_size_loc, 0x502001fc

// kernel address (move kernel to here, and jmp here to start kernel)
.equ kernel, 0x50201000

// BIOS function entry (jump here to use BIOS APIs)
.equ bios_func_entry, 0x50150000


// Address storing kernel size
.equ kernel_size_addr, 0x502001fc

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// TODO: [p1-task1] call BIOS to print string "It's bootblock!"
	la a0,msg
	li a7,BIOS_PUTSTR
	call bios_func_entry

	// TODO: [p1-task2] call BIOS to read kernel in SD card
	la a0,0x50201000
	la a1,kernel_size_addr
	lh a1,0(a1)
	li a2,1
	li a7,BIOS_SDREAD
	call bios_func_entry


	// TODO: [p1-task4] load task-related arguments and pass them to kernel
	// xjs: I do not think it is goot to let bootloader to load task-related arguments, it should be done by kernel

	// TODO: [p1-task2] jump to kernel to start UCAS-OS
	j kernel

	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's Xujinsheng's bootloader...\n\r"

