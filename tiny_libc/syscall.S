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
DEFINE_SYSCALL(sys_set_sche_workload, SYSCALL_SET_SCHE_WORKLOAD)



/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/