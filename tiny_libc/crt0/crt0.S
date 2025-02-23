#include <asm.h>

.section ".entry_function","ax"

.equ STACK_BASE, 0x200000
ENTRY(_start)
/* a7: thread id in process
main thread: a0: argc, a1: argv
child thread: a0: function pointer, a1: argument
*/
	/* setup C runtime environment for the user program */
	li gp,0
	li tp,0
	bnez a7,secondary

	// clear BSS, make sure the bss is 8-byte aligned
	la t0,__bss_start
	la t1,__BSS_END__
	beq t0,t1,2f
1:
	sd zero,0(t0)
	addi t0,t0,8
	blt t0,t1,1b
2:
	// set sp for different thread
	la sp, STACK_BASE

	mv s0, a0
	mv s1, a1

	call __crt_init

	mv a0, s0
	mv a1, s1

	/* enter main function */
	call main
	j finish
	
secondary:
	// set sp = STACK_BASE - 4096*8 * a7
	la sp, STACK_BASE
	li t0, 4096*8
	mul t0, a7, t0
	sub sp, sp, t0
	mv t0, a0
	mv a0, a1
	jalr t0



finish:
	/* finish task and return to the kernel */
	call sys_exit_thread

	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) loop, unreachable here
loop:
	wfi
	j loop

END(_start)
