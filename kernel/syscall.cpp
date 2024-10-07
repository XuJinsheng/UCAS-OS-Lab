#include <common.h>
#include <drivers/screen.h>
#include <schedule.hpp>
#include <syscall.hpp>
#include <thread.hpp>

void init_syscall()
{
}
ptr_t handle_syscall(const ptr_t args[8])
{
	ptr_t ret = 0;
	switch (args[7])
	{
	case SYSCALL_SLEEP:
		Syscall::sleep(args[0]);
		break;
	case SYSCALL_YIELD:
		Syscall::yield();
		break;
	case SYSCALL_WRITE:
		Syscall::write((char *)args[0]);
		break;
	case SYSCALL_MOVE_CURSOR:
		Syscall::move_cursor(args[0], args[1]);
		break;
	case SYSCALL_REFLUSH:
		Syscall::reflush();
		break;
	case SYSCALL_GET_TIMEBASE:
		ret = Syscall::get_timebase();
		break;
	case SYSCALL_GET_TICK:
		ret = Syscall::get_tick();
		break;
	case SYSCALL_MUTEX_INIT:
		ret = Syscall::mutex_init(args[0]);
		break;
	case SYSCALL_MUTEX_ACQ:
		Syscall::mutex_acquire(args[0]);
		break;
	case SYSCALL_MUTEX_RELEASE:
		Syscall::mutex_release(args[0]);
		break;
	case SYSCALL_SET_SCHE_WORKLOAD:
		Syscall::set_sche_workload(args[0]);
		break;
	}
	return ret;
}

void Syscall::yield(void)
{
	do_scheduler();
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

long Syscall::get_timebase(void)
{
	return 0;
}

long Syscall::get_tick(void)
{
	return 0;
}

void Syscall::sleep(uint32_t time)
{
}

void Syscall::set_sche_workload(int workload)
{
}