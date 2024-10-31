#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <drivers/screen.h>
#include <fs/fs.hpp>
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
#define SYSCALL_TASKSET 8
#define SYSCALL_CREATE_THREAD 10
#define SYSCALL_WAIT_THREAD 11
#define SYSCALL_EXIT_THREAD 12
#define SYSCALL_KILL_THREAD 13
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
#define SYSCALL_SHM_GET 56
#define SYSCALL_SHM_DT 57
#define SYSCALL_BRK 59
#define SYSCALL_SBRK 60
#define SYSCALL_NET_SEND 63
#define SYSCALL_NET_RECV 64
#define SYSCALL_FS_MKFS 65
#define SYSCALL_FS_STATFS 66
#define SYSCALL_FS_CD 67
#define SYSCALL_FS_MKDIR 68
#define SYSCALL_FS_RMDIR 69
#define SYSCALL_FS_LS 70
#define SYSCALL_FS_TOUCH 71
#define SYSCALL_FS_CAT 72
#define SYSCALL_FS_FOPEN 73
#define SYSCALL_FS_FREAD 74
#define SYSCALL_FS_FWRITE 75
#define SYSCALL_FS_FCLOSE 76
#define SYSCALL_FS_LN 77
#define SYSCALL_FS_RM 78
#define SYSCALL_FS_LSEEK 79

typedef ptr_t (*syscall_func)(ptr_t a0, ptr_t a1, ptr_t a2, ptr_t a3, ptr_t a4, ptr_t a5, ptr_t a6);
constexpr int SYSCALL_NUM = 96;
syscall_func syscall_table[SYSCALL_NUM];
void init_syscall()
{
	syscall_table[SYSCALL_EXEC] = (syscall_func)Syscall::sys_exec;
	syscall_table[SYSCALL_EXIT] = (syscall_func)Syscall::sys_exit;
	syscall_table[SYSCALL_SLEEP] = (syscall_func)Syscall::sleep;
	syscall_table[SYSCALL_KILL] = (syscall_func)Syscall::sys_kill;
	syscall_table[SYSCALL_WAITPID] = (syscall_func)Syscall::sys_waitpid;
	syscall_table[SYSCALL_PS] = (syscall_func)Syscall::sys_ps;
	syscall_table[SYSCALL_GETPID] = (syscall_func)Syscall::sys_getpid;
	syscall_table[SYSCALL_YIELD] = (syscall_func)Syscall::yield;
	syscall_table[SYSCALL_TASKSET] = (syscall_func)Syscall::sys_task_set;
	syscall_table[SYSCALL_CREATE_THREAD] = (syscall_func)Syscall::sys_create_thread;
	syscall_table[SYSCALL_WAIT_THREAD] = (syscall_func)Syscall::sys_wait_thread;
	syscall_table[SYSCALL_EXIT_THREAD] = (syscall_func)Syscall::sys_exit_thread;
	syscall_table[SYSCALL_KILL_THREAD] = (syscall_func)Syscall::sys_kill_thread;
	syscall_table[SYSCALL_WRITE] = (syscall_func)Syscall::write;
	syscall_table[SYSCALL_READCH] = (syscall_func)Syscall::sys_getchar;
	syscall_table[SYSCALL_MOVE_CURSOR] = (syscall_func)Syscall::move_cursor;
	syscall_table[SYSCALL_REFLUSH] = (syscall_func)Syscall::reflush;
	syscall_table[SYSCALL_CLEAR] = (syscall_func)Syscall::sys_clear;
	syscall_table[SYSCALL_GET_TIMEBASE] = (syscall_func)Syscall::get_timebase;
	syscall_table[SYSCALL_GET_TICK] = (syscall_func)Syscall::get_tick;
	syscall_table[SYSCALL_MUTEX_INIT] = (syscall_func)Syscall::mutex_init;
	syscall_table[SYSCALL_MUTEX_ACQ] = (syscall_func)Syscall::mutex_acquire;
	syscall_table[SYSCALL_MUTEX_RELEASE] = (syscall_func)Syscall::mutex_release;
	syscall_table[SYSCALL_BARR_INIT] = (syscall_func)Syscall::sys_barrier_init;
	syscall_table[SYSCALL_BARR_WAIT] = (syscall_func)Syscall::sys_barrier_wait;
	syscall_table[SYSCALL_BARR_DESTROY] = (syscall_func)Syscall::sys_barrier_destroy;
	syscall_table[SYSCALL_COND_INIT] = (syscall_func)Syscall::sys_condition_init;
	syscall_table[SYSCALL_COND_WAIT] = (syscall_func)Syscall::sys_condition_wait;
	syscall_table[SYSCALL_COND_SIGNAL] = (syscall_func)Syscall::sys_condition_signal;
	syscall_table[SYSCALL_COND_BROADCAST] = (syscall_func)Syscall::sys_condition_broadcast;
	syscall_table[SYSCALL_COND_DESTROY] = (syscall_func)Syscall::sys_condition_destroy;
	syscall_table[SYSCALL_MBOX_OPEN] = (syscall_func)Syscall::sys_mbox_open;
	syscall_table[SYSCALL_MBOX_CLOSE] = (syscall_func)Syscall::sys_mbox_close;
	syscall_table[SYSCALL_MBOX_SEND] = (syscall_func)Syscall::sys_mbox_send;
	syscall_table[SYSCALL_MBOX_RECV] = (syscall_func)Syscall::sys_mbox_recv;
	syscall_table[SYSCALL_SHM_GET] = (syscall_func)Syscall::sys_shmpageget;
	syscall_table[SYSCALL_SHM_DT] = (syscall_func)Syscall::sys_shmpagedt;
	syscall_table[SYSCALL_BRK] = (syscall_func)Syscall::sys_brk;
	syscall_table[SYSCALL_SBRK] = (syscall_func)Syscall::sys_sbrk;
	syscall_table[SYSCALL_NET_SEND] = (syscall_func)Syscall::sys_net_send;
	syscall_table[SYSCALL_NET_RECV] = (syscall_func)Syscall::sys_net_recv;
	syscall_table[SYSCALL_FS_MKFS] = (syscall_func)fs_mkfs;
	syscall_table[SYSCALL_FS_STATFS] = (syscall_func)fs_statfs;
	syscall_table[SYSCALL_FS_CD] = (syscall_func)fs_cd;
	syscall_table[SYSCALL_FS_LS] = (syscall_func)fs_ls;
	syscall_table[SYSCALL_FS_MKDIR] = (syscall_func)fs_mkdir;
	syscall_table[SYSCALL_FS_TOUCH] = (syscall_func)fs_touch;
	/*
	syscall_table[SYSCALL_FS_RMDIR] = (syscall_func)fs_rmdir;
	syscall_table[SYSCALL_FS_RM] = (syscall_func)fs_rm;
	syscall_table[SYSCALL_FS_LN] = (syscall_func)fs_ln;
	syscall_table[SYSCALL_FS_CAT] = (syscall_func)fs_cat;
	syscall_table[SYSCALL_FS_FOPEN] = (syscall_func)fs_fopen;
	syscall_table[SYSCALL_FS_FREAD] = (syscall_func)fs_fread;
	syscall_table[SYSCALL_FS_FWRITE] = (syscall_func)fs_fwrite;
	syscall_table[SYSCALL_FS_FCLOSE] = (syscall_func)fs_fclose;
	syscall_table[SYSCALL_FS_LSEEK] = (syscall_func)fs_lseek;
	*/
}

ptr_t handle_syscall(const ptr_t args[8])
{
	if (args[7] >= SYSCALL_NUM)
		return 0;
	ptr_t ret = 0;
	syscall_func func = syscall_table[args[7]];
	assert(func != nullptr);
	ret = func(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
	return ret;
}

int Syscall::sys_getchar()
{
	int c = bios_getchar();
	while (c == -1)
	{
		disable_preempt();
		c = bios_getchar();
		enable_preempt();
	}
	disable_preempt();
	return c;
}

void Syscall::move_cursor(int x, int y)
{
	assert_no_preempt();
	screen_move_cursor(x, y);
}

void Syscall::write(const char *buff)
{
	assert_no_preempt();
	screen_write(buff);
}

void Syscall::reflush(void)
{
	assert_no_preempt();
	screen_reflush();
}

void Syscall::sys_clear()
{
	assert_no_preempt();
	screen_clear();
}

void Syscall::sys_ps(int process, int killed)
{
	if (process)
	{
		print_processes(killed);
	}
	else
	{
		print_threads(killed);
	}
}