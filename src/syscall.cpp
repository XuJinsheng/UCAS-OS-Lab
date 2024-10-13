#include <assert.h>
#include <common.h>
#include <drivers/screen.h>
#include <schedule.hpp>
#include <syscall.hpp>
#include <thread.hpp>

#define SYSCALL_EXEC 0
#define SYSCALL_EXIT 1
#define SYSCALL_SLEEP 2
#define SYSCALL_KILL 3
#define SYSCALL_WAITPID 4
#define SYSCALL_PS 5
#define SYSCALL_GETPID 6
#define SYSCALL_YIELD 7
#define SYSCALL_WRITE 20
#define SYSCALL_READCH 21
#define SYSCALL_MOVE_CURSOR 22
#define SYSCALL_REFLUSH 23
#define SYSCALL_CLEAR 24
#define SYSCALL_GET_TIMEBASE 30
#define SYSCALL_GET_TICK 31
#define SYSCALL_SEMA_INIT 36
#define SYSCALL_SEMA_UP 37
#define SYSCALL_SEMA_DOWN 38
#define SYSCALL_SEMA_DESTROY 39
#define SYSCALL_MUTEX_INIT 40
#define SYSCALL_MUTEX_ACQ 41
#define SYSCALL_MUTEX_RELEASE 42
#define SYSCALL_SHOW_TASK 43
#define SYSCALL_BARR_INIT 44
#define SYSCALL_BARR_WAIT 45
#define SYSCALL_BARR_DESTROY 46
#define SYSCALL_COND_INIT 47
#define SYSCALL_COND_WAIT 48
#define SYSCALL_COND_SIGNAL 49
#define SYSCALL_COND_BROADCAST 50
#define SYSCALL_COND_DESTROY 51
#define SYSCALL_MBOX_OPEN 52
#define SYSCALL_MBOX_CLOSE 53
#define SYSCALL_MBOX_SEND 54
#define SYSCALL_MBOX_RECV 55

typedef ptr_t (*syscall_func)(ptr_t a0, ptr_t a1, ptr_t a2, ptr_t a3, ptr_t a4, ptr_t a5, ptr_t a6);
constexpr int SYSCALL_NUM = 96;
syscall_func syscall_table[SYSCALL_NUM];
void init_syscall()
{
	/* syscall_table[SYSCALL_EXEC] = (syscall_func)Syscall::sys_exec;
	syscall_table[SYSCALL_EXIT] = (syscall_func)Syscall::sys_exit; */
	syscall_table[SYSCALL_SLEEP] = (syscall_func)Syscall::sleep;
	/* syscall_table[SYSCALL_KILL] = (syscall_func)Syscall::sys_kill;
	syscall_table[SYSCALL_WAITPID] = (syscall_func)Syscall::sys_waitpid;
	syscall_table[SYSCALL_PS] = (syscall_func)Syscall::sys_ps;
	syscall_table[SYSCALL_GETPID] = (syscall_func)Syscall::sys_getpid; */
	syscall_table[SYSCALL_YIELD] = (syscall_func)Syscall::yield;
	syscall_table[SYSCALL_WRITE] = (syscall_func)Syscall::write;
	syscall_table[SYSCALL_MOVE_CURSOR] = (syscall_func)Syscall::move_cursor;
	syscall_table[SYSCALL_REFLUSH] = (syscall_func)Syscall::reflush;
	syscall_table[SYSCALL_GET_TIMEBASE] = (syscall_func)Syscall::get_timebase;
	syscall_table[SYSCALL_GET_TICK] = (syscall_func)Syscall::get_tick;
	syscall_table[SYSCALL_MUTEX_INIT] = (syscall_func)Syscall::mutex_init;
	syscall_table[SYSCALL_MUTEX_ACQ] = (syscall_func)Syscall::mutex_acquire;
	syscall_table[SYSCALL_MUTEX_RELEASE] = (syscall_func)Syscall::mutex_release;
}

ptr_t handle_syscall(const ptr_t args[8])
{
	if (args[7] >= SYSCALL_NUM)
		return 0;
	ptr_t ret = 0;
	syscall_func func = syscall_table[args[7]];
	if (func)
		ret = func(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
	return ret;
}

void Syscall::move_cursor(int x, int y)
{
	screen_move_cursor(x, y);
}

void Syscall::write(char *buff)
{
	screen_write(buff);
}

void Syscall::reflush(void)
{
	screen_reflush();
}
