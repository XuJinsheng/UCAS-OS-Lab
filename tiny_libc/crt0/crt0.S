#include <asm.h>

.section ".entry_function","ax"
ENTRY(_start)

    /* TODO: [p1-task3] setup C runtime environment for the user program */
    // clear BSS
    la t0,__bss_start
	la t1,__BSS_END__
	beq t0,t1,2$
1$:
	// make sure the bss is 8-byte aligned
	sd zero,0(t0)
	addi t0,t0,8
	bne t0,t1,1$
2$:
    // TODO: I do not know where is the stack pointer, so I just use the same stack pointer as kernel

    /* TODO: [p1-task3] enter main function */
    addi sp,sp,-8
    sd ra,0(sp)
    call main
    


    /* TODO: [p1-task3] finish task and return to the kernel, replace this in p3-task2! */
    ld ra,0(sp)
    addi sp,sp,8
    ret

    /************************************************************/
    /* Do not touch this comment. Reserved for future projects. */
    /************************************************************/
// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
