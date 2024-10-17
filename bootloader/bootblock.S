#include <biosdef.h>
#include <CSR.h>

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
    // a0 is mhartid
	bnez a0, secondary

	// call BIOS to print string "It's bootblock!"
	la a0,msg
	li a7,BIOS_PUTSTR
	call bios_func_entry

	// call BIOS to read kernel in SD card
	la a0,0x50201000
	la a1,kernel_size_addr
	lh a1,0(a1)
	li a2,1
	li a7,BIOS_SDREAD
	call bios_func_entry

	// jump to kernel to start UCAS-OS
    // a0 is mhartid
	la a0, 0
	wfi
	j kernel

secondary:
	csrwi sie, SIE_SSIE
	csrsi sstatus, SR_SIE
	la t0, kernel
	csrw stvec, t0
	wfi
	j secondary

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's Xujinsheng's bootloader...\n\r"
msgs: .string "It's secondary bootblock!\n\r"

