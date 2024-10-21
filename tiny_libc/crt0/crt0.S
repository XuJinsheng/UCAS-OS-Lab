#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)
    /* TODO: [p1-task3] setup C runtime environment for the user program */
    li gp,0
    li tp,0
    // clear BSS, make sure the bss is 8-byte aligned
    la t0,__bss_start
	la t1,__BSS_END__
	beq t0,t1,2$
1$:
	sd zero,0(t0)
	addi t0,t0,8
	blt t0,t1,1$
2$:
    // set sp for different thread
    la sp, _start
    li t0, 4096
    sub sp, sp, t0

    /* enter main function */
    call main
    


    /* finish task and return to the kernel */
    call sys_exit

    /************************************************************/
    /* Do not touch this comment. Reserved for future projects. */
    /************************************************************/
// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
