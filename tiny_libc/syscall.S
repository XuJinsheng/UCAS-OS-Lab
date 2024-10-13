#include <syscall.h>

#define DEFINE_SYSCALL(name,code) \
	.globl name;    \
	.balign 4;      \
	name: \
        li a7, code; \
        ecall; \
        ret;


.section .text

DEFINE_SYSCALL(sys_sleep, SYSCALL_SLEEP)
DEFINE_SYSCALL(sys_yield, SYSCALL_YIELD)
DEFINE_SYSCALL(sys_write, SYSCALL_WRITE)
DEFINE_SYSCALL(sys_move_cursor, SYSCALL_MOVE_CURSOR)
DEFINE_SYSCALL(sys_reflush, SYSCALL_REFLUSH)
DEFINE_SYSCALL(sys_get_timebase, SYSCALL_GET_TIMEBASE)
DEFINE_SYSCALL(sys_get_tick, SYSCALL_GET_TICK)
DEFINE_SYSCALL(sys_mutex_init, SYSCALL_MUTEX_INIT)
DEFINE_SYSCALL(sys_mutex_acquire, SYSCALL_MUTEX_ACQ)
DEFINE_SYSCALL(sys_mutex_release, SYSCALL_MUTEX_RELEASE)


.globl sys_getchar
.balign 4
sys_getchar:
1:
	li a7, SYSCALL_READCH
	ecall
	li a1, -1
	beq a0, a1, 1b
	ret
DEFINE_SYSCALL(sys_ps, SYSCALL_PS)
DEFINE_SYSCALL(sys_exec, SYSCALL_EXEC)
DEFINE_SYSCALL(sys_exit, SYSCALL_EXIT)
DEFINE_SYSCALL(sys_kill, SYSCALL_KILL)
DEFINE_SYSCALL(sys_waitpid, SYSCALL_WAITPID)
DEFINE_SYSCALL(sys_getpid, SYSCALL_GETPID)
DEFINE_SYSCALL(sys_barrier_init, SYSCALL_BARR_INIT)
DEFINE_SYSCALL(sys_barrier_wait, SYSCALL_BARR_WAIT)
DEFINE_SYSCALL(sys_barrier_destroy, SYSCALL_BARR_DESTROY)
DEFINE_SYSCALL(sys_condition_init, SYSCALL_COND_INIT)
DEFINE_SYSCALL(sys_condition_wait, SYSCALL_COND_WAIT)
DEFINE_SYSCALL(sys_condition_signal, SYSCALL_COND_SIGNAL)
DEFINE_SYSCALL(sys_condition_broadcast, SYSCALL_COND_BROADCAST)
DEFINE_SYSCALL(sys_condition_destroy, SYSCALL_COND_DESTROY)
DEFINE_SYSCALL(sys_mbox_open, SYSCALL_MBOX_OPEN)
DEFINE_SYSCALL(sys_mbox_close, SYSCALL_MBOX_CLOSE)
DEFINE_SYSCALL(sys_mbox_send, SYSCALL_MBOX_SEND)
DEFINE_SYSCALL(sys_mbox_recv, SYSCALL_MBOX_RECV)


/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/